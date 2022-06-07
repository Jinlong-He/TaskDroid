#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {
    ID MultiTaskAnalyzer::getTaskID(Activity* activity) {
        return a -> getTaskID(activity -> getAffinity());
    }

    void MultiTaskAnalyzer::getOutActivities() {
        auto activity = a -> getMainActivity();
        auto taskID = getTaskID(activity);
        visited[taskID].insert(activity);
        getOutActivities(taskID, activity, visited);
    }

    void MultiTaskAnalyzer::getOutActivities(ID taskID, Activity* activity,
                                             unordered_map<ID, Activities>& visited) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            auto newTaskID = getTaskID(newActivity);
            if (AndroidStackMachine::isNewMode(activity, intent) && 
                taskID != newTaskID ||
                AndroidStackMachine::getMode(activity, intent) == MKTK ||
                AndroidStackMachine::getMode(activity, intent) == SIST) {
                outActivities[taskID].insert(activity);
                if (visited[newTaskID].insert(newActivity).second)
                    getOutActivities(newTaskID, newActivity, visited);
            } else {
                if (visited[taskID].insert(newActivity).second)
                    getOutActivities(taskID, newActivity, visited);
            }
        }
    }

    void MultiTaskAnalyzer::mkLegalPos() {
        auto activity = a -> getMainActivity();
        auto taskID = getTaskID(activity);
        getOutActivities();
        if (hasRTOF) {
            for (auto [taskID, acts] : visited) {
                for (auto act : acts) {
                    if (act -> getLaunchMode() == SIT) {
                        legalPos[act][taskID].insert(0);
                    } else {
                        for (ID j = 0; j < k; j++) {
                            legalPos[act][taskID].insert(j);
                        }
                    }
                }
            }
        } else {
            legalPos[activity][taskID].insert(0);
            legalPosSize = 1;
            mkLegalPos(activity, taskID, 0);
        }
        for (auto& [taskID, mTaskID] : multiTaskIDMap) {
            for (auto& [act, map] : legalPos) {
                if (map.count(taskID) > 0) {
                    auto& set = map.at(taskID);
                    for (ID i = 0; i < m; i++) {
                        map[mTaskID + i] = set;
                    }
                }
            }
        }
        //for (auto& [act, map] : legalPos) {
        //    cout << act -> getName() << endl;
        //    for (auto& [id, set] : map) {
        //        cout << id << endl;
        //        for (auto id : set) 
        //            cout << id << " ";
        //        cout << endl;
        //    }
        //}
    }

    void MultiTaskAnalyzer::mkLegalPos(Activity* activity, ID taskID, ID actID) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            auto newTaskID = getTaskID(newActivity);
            auto mode = AndroidStackMachine::getMode(activity, intent);
            if (mode == PUSH ||
                mode == STOP && activity != newActivity ||
                mode == CTOP && activity != newActivity ||
                mode == PUSH_N && taskID == newTaskID ||
                mode == STOP_N && activity != newActivity && taskID == newTaskID ||
                mode == CTOP_N && activity != newActivity && taskID == newTaskID) {
                if (finish) {
                    if (legalPosSize != records[newActivity][taskID][actID]) {
                        if (legalPos[newActivity][taskID].insert(actID).second)
                            legalPosSize += 1;
                        records[newActivity][taskID][actID] = legalPosSize;
                        mkLegalPos(newActivity, taskID, actID);
                    }
                } else {
                    if (actID >= k - 1) continue;
                    if (legalPosSize != records[newActivity][taskID][actID + 1]) {
                        if (legalPos[newActivity][taskID].insert(actID + 1).second)
                            legalPosSize += 1;
                        records[newActivity][taskID][actID + 1] = legalPosSize;
                        mkLegalPos(newActivity, taskID, actID + 1);
                    }
                }
            } else if (mode == CTSK) {
                if (legalPosSize != records[newActivity][taskID][0]) {
                    if (legalPos[newActivity][taskID].insert(0).second)
                        legalPosSize += 1;
                    records[newActivity][taskID][0] = legalPosSize;
                    mkLegalPos(newActivity, actID, 0);
                }
            } else if (mode == PUSH_N && taskID != newTaskID ||
                       mode == STOP_N && taskID != newTaskID ||
                       mode == CTOP_N && taskID != newTaskID) {
                if (legalPosSize != records[newActivity][newTaskID][0]) {
                    if (legalPos[newActivity][newTaskID].insert(0).second)
                        legalPosSize += 1;
                    records[newActivity][newTaskID][0] = legalPosSize;
                    mkLegalPos(newActivity, newTaskID, 0);
                }
                if (outActivities.count(newTaskID) == 0) continue;
                unordered_set<ID> poses;
                for (auto out : outActivities.at(newTaskID)) {
                    if (legalPos.count(out) == 0) continue;
                    for (auto& [tid, aids] : legalPos.at(out)) {
                        for (auto aid : aids)
                            if (tid == newTaskID) poses.insert(aid);
                    }
                }
                for (auto pos : poses) {
                    if (pos == k - 1) continue;
                    if (legalPosSize != records[newActivity][newTaskID][pos + 1]) {
                        if (legalPos[newActivity][newTaskID].insert(pos + 1).second)
                            legalPosSize += 1;
                        records[newActivity][newTaskID][pos + 1] = legalPosSize;
                        mkLegalPos(newActivity, newTaskID, pos + 1);
                    }
                }
            } else if (mode == CTSK_N && taskID != newTaskID || 
                       mode == SIST ||
                       mode == MKTK) {
                if (legalPosSize != records[newActivity][newTaskID][0]) {
                    if (legalPos[newActivity][newTaskID].insert(0).second)
                        legalPosSize += 1;
                    records[newActivity][newTaskID][0] = legalPosSize;
                    mkLegalPos(newActivity, newTaskID, 0);
                }
            } 
        }
    }
}

#include <iostream>
#include <queue>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
namespace TaskDroid {
    void getRealActivities(AndroidStackMachine* a,
                           string& affinity,
                           Activity* activity,
                           unordered_map<string, Activities>& realActivities,
                           unordered_map<string, Activities>& visited) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            string newAffinity = newActivity -> getAffinity();
            if (AndroidStackMachine::isNewMode(intent) && 
                newAffinity != affinity) {
                if (realActivities[newAffinity].insert(newActivity).second) {
                    visited[newAffinity].insert(newActivity);
                    getRealActivities(a, newAffinity, newActivity, 
                                      realActivities, visited);
                }
            } else {
                if (visited[affinity].insert(newActivity).second) {
                    getRealActivities(a, affinity, newActivity, 
                                      realActivities, visited);
                }
            }
        }
    }

    void getRealActivities(AndroidStackMachine* a,
                           unordered_map<string, Activities>& realActivities) {
        auto mainActivity = a -> getMainActivity();
        string mainAffinity = mainActivity -> getAffinity();
        realActivities[mainAffinity].insert(a -> getMainActivity());
        unordered_map<string, Activities> visited;
        visited[mainAffinity].insert(mainActivity);
        getRealActivities(a, mainAffinity, mainActivity, realActivities, visited);
    }

    void getPort(AndroidStackMachine*a, 
                 Activity* masterActivity, Activity* slaveActivity,
                 ActionMap& masterOutPort, ActionMap& slaveInPort,
                 ActionMap& completeActionMap) {
        Activities visited({masterActivity}), work({masterActivity}), newWork;
        auto& actionMap = a -> getActionMap();
        while(work.size() > 0) {
            for (auto activity : work) {
                if (actionMap.count(activity) == 0) continue;
                for (auto& [intent, finish] : actionMap.at(activity)) {
                    auto action = pair(intent, finish);
                    if (AndroidStackMachine::isNewMode(intent)) {
                        auto& affinity = intent -> getActivity() -> getAffinity();
                        if (affinity == slaveActivity -> getAffinity()) {
                            masterOutPort[activity].emplace_back(action);
                            slaveInPort[activity].emplace_back(action);
                        } else if (affinity == masterActivity -> getAffinity()) {
                            completeActionMap[activity].emplace_back(action);
                        }
                    } else {
                        completeActionMap[activity].emplace_back(action);
                        auto newActivity = intent -> getActivity();
                        if (visited.insert(newActivity).second) {
                            newWork.insert(newActivity);
                        }
                    }
                }
            }
            work.clear();
            if (newWork.size() > 0) {
                work = newWork;
                newWork.clear();
            }
        }
    }
    
    void getVirtualActions(ActionMap& virtualActionMap,
                           const ActionMap& outPort, const ActionMap& inPort) {
        for (auto& [oActivity, oActions] : outPort) {
            for (auto& [oIntent, oFinish] : oActions) {
                for (auto& [iActivity, iActions] : inPort) {
                    for (auto& [iIntent, iFinish] : iActions) {
                        virtualActionMap[oActivity].emplace_back(pair(iIntent, oFinish));
                    }
                }
            }
        }
    }

    void getCompleteActions(AndroidStackMachine* a, 
                            Activity* masterActivity, 
                            Activity* slaveActivity, 
                            ActionMap& masterCompleteActionMap, 
                            ActionMap& slaveCompleteActionMap) {
        ActionMap masterOutPort, masterInPort, slaveOutPort, slaveInPort,
                  masterVirtualActionMap, slaveVirtualActionMap;
        getPort(a, masterActivity, slaveActivity, masterOutPort, slaveInPort,
                masterCompleteActionMap);
        getPort(a, slaveActivity, masterActivity, slaveOutPort, masterInPort,
                slaveCompleteActionMap);
        getVirtualActions(masterVirtualActionMap, masterOutPort, masterInPort);
        getVirtualActions(slaveVirtualActionMap, slaveOutPort, slaveInPort);
        masterCompleteActionMap.insert(masterVirtualActionMap.begin(),
                                       masterVirtualActionMap.end());
        slaveCompleteActionMap.insert(slaveVirtualActionMap.begin(),
                                      slaveVirtualActionMap.end());
    }

    void getCompleteActions(AndroidStackMachine* a, 
                            unordered_map<Activity*, ActionMap>& completeActions) {
        unordered_map<string, Activities> realActivities;
        getRealActivities(a, realActivities);
        for (auto& [masterAffinity, masterActivities] : realActivities) {
            for (auto& [slaveAffinity, slaveActivities] : realActivities) {
                if (slaveAffinity == masterAffinity) continue;
                for (auto masterActivity : masterActivities) {
                    for (auto slaveActivity : slaveActivities) {
                        getCompleteActions(a, masterActivity, slaveActivity,
                                           completeActions[masterActivity],
                                           completeActions[slaveActivity]);
                    }
                }
            }
        }
    }

    void getGraph(const ActionMap& completeGraph,
                  unordered_map<Activity*, 
                                unordered_map<Activity*, int> >& graph) {
        for (auto& [activity, actions] : completeGraph) {
            for (auto& [intent, finish] : actions) {
                auto mode = AndroidStackMachine::getMode(intent);
                auto newActivity = intent -> getActivity();
                auto& map = graph[activity];
                if (mode == PUSH || mode == PUSH_N ||
                   (activity != newActivity) && 
                   (mode == CTOP || mode == CTOP_N)) {
                    if (finish) {
                        if (map.count(newActivity) == 0) {
                            map[activity] = 0;
                        } else {
                            int weight = map.at(newActivity);
                            map[newActivity] = weight > 0 ? weight : 0;
                        }
                    } else {
                        map[newActivity] = 1;
                    }
                } else if (activity == newActivity &&
                           (mode == CTOP || mode == CTOP_N)) {
                    if (finish) {
                        if (map.count(newActivity) == 0) {
                            map[activity] = -1;
                        } else {
                            int weight = map.at(newActivity);
                            map[newActivity] = weight > -1 ? weight : -1;
                        }
                    } else {
                        if (map.count(newActivity) == 0) {
                            map[activity] = 0;
                        } else {
                            int weight = map.at(newActivity);
                            map[newActivity] = weight > 0 ? weight : 0;
                        }
                    }
                }
            }
        }
    }

    bool MultiTaskAnalyzer::analyzeBoundedness() {
        unordered_map<Activity*, ActionMap> completeActions;
        getCompleteActions(a, completeActions);
        unordered_map<Activity*, unordered_map<Activity*, int> > graph;
        vector<vector<Activity*> > loops;
        for (auto& [realActivity, completeGraph] : completeActions) {
            graph.clear();
            loops.clear();
            getGraph(completeGraph, graph);
            LoopAnalyzer<Activity>::getLoop(graph, realActivity, loops);
            if (loops.size() == 0) {
                analyzeReachability(realActivity -> getAffinity(), vector<Activity*>());
                return 0;
            }
            for (auto& loop : loops) {
                analyzeReachability(realActivity -> getAffinity(), loop);
                return 0;
            }
        }
    }
}

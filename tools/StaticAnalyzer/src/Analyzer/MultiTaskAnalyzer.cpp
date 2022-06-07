#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {

    void MultiTaskAnalyzer::loadASM(AndroidStackMachine* a) {
        this -> a = a;
        this -> a -> formalize();
        taskNum = this -> a -> getAffinityMap().size();
        for (auto& [name, activity] : a -> getActivityMap())
            if (activity -> getLaunchMode() == SIT) 
                sitIDSet.insert(getTaskID(activity));
        for (ID i = 0; i < taskNum; i++)
            taskIDMap[i] = i;
        for (auto& [activity, actions] : a -> getActionMap())
            for (auto& [intent, finish]  : actions) {
                auto mode = AndroidStackMachine::getMode(activity, intent);
                if (mode == MKTK) {
                    auto taskID = getTaskID(intent -> getActivity());
                    if (multiTaskIDMap.count(taskID) > 0) continue;
                    multiTaskIDMap[taskID] = taskNum;
                    for (ID i = 0; i < m; i++)
                        taskIDMap[taskNum + i] = taskID;
                    taskNum += m;
                } else if (mode == RTOF || mode == RTOF_N) {
                    hasRTOF = 1;
                }
            }
        mkVarsValues();
        translate2FOA();
    }

    void MultiTaskAnalyzer::getPattenTaskAP(const string& affinity,
                                            const vector<Activity*>& task,
                                            atomic_proposition& ap) {
        if (task.size() > k || task.size() == 0) return;
        auto taskID = a -> getTaskID(affinity);
        for (ID i = 0; i <= k - task.size(); i++) {
            atomic_proposition p("TRUE");
            for (ID j = i; j - i < task.size(); j++) {
                auto& var = *activityVarMap.at(pair(taskID, j));
                auto& value = *activityValueMap.at(task[j - i]);
                p = p & var == value;
            }
            ap = ap | p;
        }
    }

    void MultiTaskAnalyzer::getTaskAP(const string& affinity,
                                      const vector<Activity*>& task,
                                      atomic_proposition& ap) {
        if (task.size() > k) return;
        auto taskID = a -> getTaskID(affinity);
        for (ID j = 0; j < k; j++) {
            auto& var = *activityVarMap.at(pair(taskID, j));
            if (j < task.size()) {
                auto& value = *activityValueMap.at(task[j]);
                ap = ap & var == value;
            } else {
                ap = ap & var == nullValue;
            }
        }
    }

    bool MultiTaskAnalyzer::analyzePattenReachability(const string& affinity,
                                                      const vector<Activity*>& task,
                                                      std::ostream& os) {
        atomic_proposition ap("FALSE");
        getPattenTaskAP(affinity, task, ap);
        return analyzeReachability(ap, os);
    }

    bool MultiTaskAnalyzer::analyzeReachability(const string& affinity,
                                                const vector<Activity*>& task,
                                                std::ostream& os) {
        atomic_proposition ap("TRUE");
        getTaskAP(affinity, task, ap);
        return analyzeReachability(ap, os);
    }

    bool MultiTaskAnalyzer::analyzeReachability(const atomic_proposition& ap,
                                                std::ostream& os) {
        translate2FOA();
        std::ifstream f("nuxmv_result");
        if (f) system("rm nuxmv_result");
        verify_invar_nuxmv(foa, ap, "nuxmv_source", 50);
        std::ifstream fin("nuxmv_result");
        if (!fin) return false;
        unordered_map<string, vector<string> > trace_table;
        parse_trace_nuxmv(foa, "nuxmv_result", trace_table);
        os << "-Activity Trace Found:" << endl;
        unordered_set<string> values;
        for (auto& value : trace_table.at("t")) {
            if (value == "pop") {
                os << "back" << endl;
            } else {
                os << value2ActionMap[value].first -> getName() << " -> " <<
                      value2ActionMap[value].second -> getActivity() -> getName() << endl;
                values.insert(value);
            }
        }
        os << "---Trace END---" << endl;
        //os << "-Acitivity Transition Coverage: " << 1.0 * values.size()/value2ActionMap.size() << endl;;
        //os << "-Acitivity Transition Coverage: " << values.size() << "/" << value2ActionMap.size() << endl;;
        return true;
    }
    
}


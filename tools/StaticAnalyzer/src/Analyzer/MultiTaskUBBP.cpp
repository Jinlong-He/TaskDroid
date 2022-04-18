#include <iostream>
#include <fstream>
#include <time.h> 
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;

namespace TaskDroid {
    void MultiTaskAnalyzer::getUBBPctpAP(Activity* source, Activity* target,
                                         atomic_proposition& ap, bool n) {
        auto& value = *activityValueMap.at(source);
        auto& targetValue = *activityValueMap.at(target);
        auto targetID = a -> getTaskID(target -> getAffinity());
        atomic_proposition p("FALSE");
        for (auto& [i, js] : availablePos.at(source)) {
            if (n && i != targetID) continue;
            atomic_proposition orderAP = getTopOrderAP(i);
            atomic_proposition actAP = atomic_proposition("FALSE");
            for (auto j : js) {
                auto& var = *activityVarMap.at(pair(i, j));
                auto ap = (var == value);
                if (j + 1 < k) {
                    auto& var = *activityVarMap.at(pair(i, j + 1));
                    ap = (var == nullValue) & ap;
                }
                auto targetAP = atomic_proposition("FALSE");
                for (ID jj = 0; jj < j; jj++) {
                    auto& var = *activityVarMap.at(pair(i, jj));
                    targetAP = targetAP | (var == targetValue);
                }
                if (targetAP.to_string() == "FALSE") 
                    targetAP = atomic_proposition("TRUE");
                actAP = actAP | ap & targetAP;
            }
            if (actAP.to_string() == "FALSE") actAP = atomic_proposition("TRUE");
            p = p | orderAP & actAP;
        }
        ap = ap & p;
    }
    void MultiTaskAnalyzer::getUBBPnewAP(Activity* source, Activity* target,
                                         atomic_proposition& ap) {
        auto& value = *activityValueMap.at(source);
        atomic_proposition p("FALSE");
        auto targetID = a -> getTaskID(target -> getAffinity());
        for (auto& [i, js] : availablePos.at(source)) {
            atomic_proposition orderAP = getTopOrderAP(i, targetID, 0);
            atomic_proposition actAP = atomic_proposition("FALSE");
            for (auto j : js) {
                auto& var = *activityVarMap.at(pair(i, j));
                auto ap = (var == value);
                if (j + 1 < k) {
                    auto& var = *activityVarMap.at(pair(i, j + 1));
                    ap = (var == nullValue) & ap;
                }
                actAP = actAP | ap;
            }
            if (actAP.to_string() == "FALSE") actAP = atomic_proposition("TRUE");
            p = p | orderAP & actAP;
        }
        ap = ap & p;
    }
    bool MultiTaskAnalyzer::analyzeUBBP(std::ostream& os) {
        bool flag = false;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                if (AndroidStackMachine::getMode(intent) == PUSH_N ||
                    AndroidStackMachine::getMode(intent) == STOP_N ||
                    AndroidStackMachine::getMode(intent) == CTOP_N) {
                    atomic_proposition ap("TRUE");
                    getUBBPnewAP(activity, intent -> getActivity(), ap);
                    os << "-UBBP New Patten Found:" << endl;
                    os << activity -> getName() << " -> "
                       << intent -> getActivity() -> getName() << endl;
                    os << "---Patten END---" << endl;
                    if (analyzeReachability(ap, os)) flag = true;
                } else if (AndroidStackMachine::getMode(intent) == CTOP) {
                    atomic_proposition ap("TRUE");
                    getUBBPctpAP(activity, intent -> getActivity(), ap);
                    os << "-UBBP CTP Patten Found:" << endl;
                    os << activity -> getName() << " -> "
                       << intent -> getActivity() -> getName() << endl;
                    os << "---Patten END---" << endl;
                    if (analyzeReachability(ap, os)) flag = true;
                } else if (AndroidStackMachine::getMode(intent) == CTOP_N) {
                    atomic_proposition ap("TRUE");
                    getUBBPctpAP(activity, intent -> getActivity(), ap, 1);
                    os << "-UBBP CTP Patten Found:" << endl;
                    os << activity -> getName() << " -> "
                       << intent -> getActivity() -> getName() << endl;
                    os << "---Patten END---" << endl;
                    if (analyzeReachability(ap, os)) flag = true;
                } else if (AndroidStackMachine::getMode(intent) == CTSK ||
                           finish) {
                    os << "-UBBP CTK Patten Found:" << endl;
                    os << activity -> getName() << " -> "
                       << intent -> getActivity() -> getName() << endl;
                    flag = true;
                }
            }
        }
        return flag;
    }
}

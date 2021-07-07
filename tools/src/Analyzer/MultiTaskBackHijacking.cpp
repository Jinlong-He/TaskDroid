#include <iostream>
#include <queue>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
namespace TaskDroid {

    void MultiTaskAnalyzer::getBackHijackingAP(Activity* source, 
                                               Activity* target,
                                               atomic_proposition& ap) {
        if (k < 3) return;
        auto targetTaskID = a -> getTaskID(target -> getAffinity());
        auto& var0 = *activityVarMap.at(pair(targetTaskID, 0));
        auto& var1 = *activityVarMap.at(pair(targetTaskID, 1));
        auto& var2 = *activityVarMap.at(pair(targetTaskID, 2));
        auto& targetValue = *activityValueMap.at(target);
        auto& sourceValue = *activityValueMap.at(source);
        ap = ap & (var0 != nullValue) & (var1 == targetValue) & (var2 == nullValue);
        auto sourceAP = atomic_proposition("FALSE");
        for (ID taskID = 0; taskID < taskNum; taskID++) {
            if (taskID == targetTaskID) continue;
            atomic_proposition orderAP("FALSE");
            getTopOrderAP(targetTaskID, taskID, orderAP);
            auto taskAP = atomic_proposition("FALSE");
            for (ID j = 0; j < k; j++) {
                auto& var = *activityVarMap.at(pair(taskID, j));
                auto p = var == sourceValue;
                if (j < k - 1) {
                    auto& var = *activityVarMap.at(pair(taskID, j + 1));
                    p = p & (var == nullValue);
                }
                taskAP = taskAP | p;
            }
            sourceAP = sourceAP | (orderAP & taskAP);
        }
        ap = ap & sourceAP;
    }

    bool MultiTaskAnalyzer::analyzeBackHijacking(std::ostream& os) {
        if (taskNum == 1) return false;
        unordered_map<Activity*, Activities> pattens;
        for (auto& [source, actionMap] : a -> getActionMap()) {
            for (auto& [intent, finish] : actionMap) {
                if (AndroidStackMachine::getMode(intent) == PUSH_N ||
                    AndroidStackMachine::getMode(intent) == STOP_N ||
                    AndroidStackMachine::getMode(intent) == CTOP_N) 
                    pattens[source].insert(intent -> getActivity());
            }
        }
        if (pattens.size() == 0) return false;
        for (auto& [source, targets] : pattens) {
            for (auto target : targets) {
                os << "-BackHijacking Patten Found:" << endl;
                os << source -> getName() << endl;
                os << target -> getName() << endl;
                os << "---END---" << endl;
                atomic_proposition ap("TRUE");
                getBackHijackingAP(source, target, ap);
                analyzeReachability(ap, os);
            }
        }
        return true;
    }
}

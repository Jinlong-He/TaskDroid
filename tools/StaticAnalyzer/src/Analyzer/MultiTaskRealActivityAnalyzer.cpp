#include <iostream>
#include <queue>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
namespace TaskDroid {
    void MultiTaskAnalyzer::getRealActAP(Activity* source, Activity* target,
                                         atomic_proposition& ap) {
        auto& value = *activityValueMap.at(source);
        atomic_proposition p("FALSE");
        for (auto& [i, js] : availablePos.at(source)) {
            atomic_proposition orderAP = getTopOrderAP(i);
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
        auto targetID = a -> getTaskID(target -> getAffinity());
        auto& var = *realActivityVarMap.at(targetID);
        auto& targetValue = *activityValueMap.at(target);
        ap = ap & p & (var == targetValue);
    }

    bool MultiTaskAnalyzer::analyzeRealActivity(std::ostream& os) {
        bool flag = false;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                if (AndroidStackMachine::getMode(intent) == PUSH_N ||
                    AndroidStackMachine::getMode(intent) == STOP_N) {
                    atomic_proposition ap("TRUE");
                    getRealActAP(activity, intent -> getActivity(), ap);
                    os << "-RealActivity Patten Found:" << endl;
                    os << activity -> getName() << " -> "
                       << intent -> getActivity() -> getName() << endl;
                    os << "---Patten END---" << endl;
                    if (analyzeReachability(ap, os)) flag = true;
                }
            }
        }
        return flag;
    }
}

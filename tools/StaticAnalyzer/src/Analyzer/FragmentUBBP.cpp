
#include <iostream>
#include <queue>
#include <cmath>
#include "Analyzer/FragmentAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
namespace TaskDroid {
    void FragmentAnalyzer::getUBBPAP(FragmentTransaction* transaction, 
                                     atomic_proposition& ap) {
        auto topAP = atomic_proposition("FALSE");
        auto& value = *transactionValueMap.at(transaction);
        for (ID i = 0; i < h; i++) {
            auto& var = *backTransactionVarMap.at(i);
            auto p = var == value;
            if (i < h - 1) {
                auto& newVar = *backTransactionVarMap.at(i + 1);
                p = p & newVar == nullValue;
            }
            topAP = topAP | p;
        }
        auto actionAP = atomic_proposition("TRUE");
        for (auto action : transaction -> getHighLevelFragmentActions()) {
            auto viewID = viewMap.at(action -> getViewID());
            auto& value = *fragmentActionValueMap.at(action);
            auto& map = fragmentVarMap.at(viewID);
            auto hAP = atomic_proposition("FALSE");
            for (ID i = 0; i < h; i++) {
                auto topAP = atomic_proposition("FALSE");
                getTopOrderAP(viewID, i, topAP);
                auto fragAP = atomic_proposition("TRUE");
                for (ID j = 0; j < k; j++) {
                    auto& var = *map.at(pair(i, j));
                    auto p = (var != value) & (var != nullValue);
                    if (j < k - 1) {
                        auto& newVar = *map.at(pair(i, j + 1));
                        p = p & newVar == nullValue;
                    }
                    fragAP = fragAP & p;
                }
                hAP = hAP | (topAP & fragAP);
            }
            actionAP = actionAP & hAP;
        }
        ap = topAP & actionAP;
    }

    bool FragmentAnalyzer::analyzeUBBP(std::ostream& os) {
        bool flag = false;
        for (auto& [f, transactions] : fragmentTransactionMap) {
            for (auto transaction : transactions) {
                if (!transaction -> isAddTobackStack()) continue;
                bool modeFlag = true;
                for (auto action : transaction -> getHighLevelFragmentActions()) {
                    auto ap = atomic_proposition("TRUE");
                    getUBBPAP(transaction, ap);
                    os << "-FragUBBP Patten Found:" << endl;
                    os << transaction -> toString() << endl;
                    os << "---Patten END---" << endl;
                    if (analyzeReachability(ap, os)) flag = true;
                }
            }
        }
        return flag;
    }
}

#include <iostream>
#include <fstream>
#include <time.h> 
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;

namespace TaskDroid {
    atomic_proposition MultiTaskAnalyzer::getRealActAP(Activity* source, Activity* target) {
        auto& value = *activityValueMap.at(source);
        auto targetID = getTaskID(target);
        atomic_proposition p("FALSE");
        if (legalPos.count(source) == 0) return p;
        for (auto& [i, js] : legalPos.at(source)) {
            if (js.size() == 0) continue;
            atomic_proposition orderAP = mkIsTopTaskAP(i, targetID);
            atomic_proposition actAP = atomic_proposition("FALSE");
            for (auto j : js) {
                actAP = actAP | mkIsTopActAP(i, j, source);
            }
            p = p | orderAP & actAP;
        }
        auto& targetValue = *activityValueMap.at(target);
        auto& var = *realActivityVarMap.at(targetID);
        auto ap = p & (var == targetValue);
        if (target == a -> getMainActivity()) ap = ap & mainTaskVar == bool_value(0);
        return ap;
    }

    atomic_proposition MultiTaskAnalyzer::getRTF7AP(Activity* source, Activity* target) {
        auto& value = *activityValueMap.at(source);
        auto& targetValue = *activityValueMap.at(target);
        atomic_proposition orderAP = mkIsTopTaskAP(mainTaskID);
        atomic_proposition actAP = atomic_proposition("FALSE");
        if (legalPos.count(source) == 0 || 
            legalPos.at(source).count(mainTaskID) == 0) return actAP;
        auto& js = legalPos.at(source).at(mainTaskID);
        for (ID j = 1; j < k; j++) {
            auto targetAP = atomic_proposition("FALSE");
            for (ID jj = 0; jj < j; jj++) {
                auto& var = *activityVarMap.at(pair(mainTaskID, jj));
                targetAP = targetAP | var == targetValue;
            }
            actAP = actAP | mkIsTopActAP(mainTaskID, j, source) & targetAP;
        }
        auto& var = *realActivityVarMap.at(mainTaskID);
        auto ap = orderAP & actAP & var == mainActivityValue & mainTaskVar == bool_value(1);
        return ap;
    }

    atomic_proposition MultiTaskAnalyzer::getRTF11AP(Activity* source, Activity* target) {
        auto& value = *activityValueMap.at(source);
        auto& targetValue = *activityValueMap.at(target);
        auto targetID = getTaskID(target);
        atomic_proposition p("FALSE"), targetActAP = atomic_proposition("FALSE");
        if (legalPos.count(target) == 0 ||
            legalPos.at(target).count(targetID) == 0) return p;
        for (auto j : legalPos.at(target).at(targetID)) {
            auto& var = *activityVarMap.at(pair(targetID, j));
            targetActAP = targetActAP | var == targetValue;
        }
        if (legalPos.count(source) == 0) return p;
        for (auto& [i, js] : legalPos.at(source)) {
            atomic_proposition orderAP = mkIsTopTaskAP(i, targetID);
            atomic_proposition actAP = atomic_proposition("FALSE");
            if (js.size() == 0) continue;
            for (auto j : js) {
                actAP = actAP | mkIsTopActAP(i, j, source);
            }
            p = p | orderAP & actAP;
        }
        return p & targetActAP;
    }

    atomic_proposition MultiTaskAnalyzer::getSTPAP(Activity* source, Activity* target) {
        auto& value = *activityValueMap.at(source);
        auto& targetValue = *activityValueMap.at(target);
        auto ap = atomic_proposition("FALSE");
        if (legalPos.count(source) == 0) return ap;
        for (auto& [i, js] : legalPos.at(source)) {
            atomic_proposition orderAP = mkIsTopTaskAP(i);
            atomic_proposition actAP = atomic_proposition("FALSE");
            if (js.size() == 0) continue;
            for (auto j : js) {
                auto targetAP = atomic_proposition("FALSE");
                for (ID jj = 0; jj < j; jj++) {
                    auto& var = *activityVarMap.at(pair(mainTaskID, jj));
                    targetAP = targetAP | var == targetValue;
                }
                actAP = actAP | mkIsTopActAP(i, j, source) & targetAP;
            }
            ap = ap | orderAP & actAP;
        }
        return ap;
    }

    bool MultiTaskAnalyzer::analyzeUnexpectedness(std::ostream& os) {
        bool flag = false;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                auto target = intent -> getActivity();
                auto lmd = target -> getLaunchMode();
                auto mode = AndroidStackMachine::getMode(activity, intent);
                if (mode == PUSH_N || mode == STOP_N) {
                    os << "[Unexpectedness: REAL(all)] Pattern Found:" << endl;
                    os << activity -> getName() << " -> "
                       << target -> getName() << endl;
                    os << "---Pattern END---" << endl;
                    atomic_proposition ap = getRealActAP(activity, target);
                    if (analyzeReachability(ap, os)) flag = true;
                } 
                if ((mode == RTOF || mode == RTOF_N) && activity != target) {
                    os << "[Unexpectedness: RTF(7.0)] Pattern Found:" << endl;
                    os << activity -> getName() << " -> "
                       << target -> getName() << endl;
                    os << "---Pattern END---" << endl;
                    atomic_proposition ap = getRTF7AP(activity, target);
                    if (analyzeReachability(ap, os)) flag = true;
                }
                if (mode == RTOF_N) {
                    os << "[Unexpectedness: RTF(11.0*)] Pattern Found:" << endl;
                    os << activity -> getName() << " -> "
                       << target -> getName() << endl;
                    os << "---Pattern END---" << endl;
                    atomic_proposition ap = getRTF11AP(activity, target) |
                                            getRealActAP(activity, target);
                    if (analyzeReachability(ap, os)) flag = true;
                }
                if (mode == CTOP &&
                    (target -> getLaunchMode() == STP || 
                     intent -> getFlags().count(F_STP))) {
                    os << "[Unexpectedness: STP(all)] Pattern Found:" << endl;
                    os << activity -> getName() << " -> "
                       << target -> getName() << endl;
                    os << "---Pattern END---" << endl;
                    atomic_proposition ap = getSTPAP(activity, target);
                    if (analyzeReachability(ap, os)) flag = true;
                }
                if (mode == CTOP_N && lmd != STK && 
                    (lmd == STP || intent -> getFlags().count(F_STP))) {
                    os << "[Unexpectedness: STPN(all)] Pattern Found:" << endl;
                    os << activity -> getName() << " -> "
                       << target -> getName() << endl;
                    os << "---Pattern END---" << endl;
                    atomic_proposition ap = getRTF11AP(activity, target);
                    if (analyzeReachability(ap, os)) flag = true;
                }
            }
        }
        return flag;
    }
}


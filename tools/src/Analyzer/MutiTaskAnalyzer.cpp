#include <iostream>
#include "Analyzer/MutiTaskAnalyzer.hpp"
#include "fml/atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
using namespace atl;
using namespace ll;
namespace TaskDroid {
    void MutiTaskAnalyzer::analyzeBoundedness(AndroidStackMachine* a) {
    }

    vector<string> getOrders(int n) {

    }

    void mkPUSH(Activity* activity, Intent* intent, ID taskId, ID actId, ID k,
                const unordered_map<pair<ID, ID>, enum_variable*>& activityVarMap,
                const unordered_map<Activity*, enum_value*>& activityValueMap,
                const atomic_proposition& ap,
                fomula_automaton<>& foa) {
        auto& var = *activityVarMap.at(pair(taskId, actId));
        auto& value = *activityValueMap.at(activity);
        auto& newValue = *activityValueMap.at(intent -> getActivity());
        if (actId < k - 1) {
            auto& newVar = *activityVarMap.at(pair(taskId, actId + 1));
            add_transition(foa, newVar, newValue, (var == value) & ap);
        } 
    }

    void mkSTOP(Activity* activity, Intent* intent, ID taskId, ID actId, ID k,
                const unordered_map<pair<ID, ID>, enum_variable*>& activityVarMap,
                const unordered_map<Activity*, enum_value*>& activityValueMap,
                const atomic_proposition& ap,
                fomula_automaton<>& foa) {
        if (activity == intent -> getActivity()) return;
        mkPUSH(activity, intent, taskId, actId, k, 
               activityVarMap, activityValueMap, ap, foa);
    }

    void mkCTOP(Activity* activity, Intent* intent, ID taskId, ID actId, ID k,
                const unordered_map<pair<ID, ID>, enum_variable*>& activityVarMap,
                const unordered_map<Activity*, enum_value*>& activityValueMap,
                const atomic_proposition& ap,
                fomula_automaton<>& foa) {
        if (activity == intent -> getActivity()) return;
        enum_value nullValue("null");
        auto& var = *activityVarMap.at(pair(taskId, actId));
        auto& value = *activityValueMap.at(activity);
        auto& newValue = *activityValueMap.at(intent -> getActivity());
        auto topAP = (var == value);
        for (ID i = 0; i < actId; i++) {
            auto& newVar = *activityVarMap.at(pair(taskId, i));
            auto p = (var == value) & (newVar == newValue);
            for (ID j = i + 1; j < actId; j++) {
                p = p & (*activityVarMap.at(pair(taskId, j)) != newValue);
            }
            for (ID j = i + 1; j <= actId; j++) {
                auto& var = *activityVarMap.at(pair(taskId, j));
                add_transition(foa, var, nullValue, p & ap);
            }
            topAP = topAP & (newVar != newValue);
        }
        if (actId < k - 1) {
            auto& newVar = *activityVarMap.at(pair(taskId, actId + 1));
            add_transition(foa, newVar, newValue, topAP & ap);
        } 
    }

    void mkCTSK(Activity* activity, Intent* intent, ID taskId, ID actId, ID k,
                const unordered_map<pair<ID, ID>, enum_variable*>& activityVarMap,
                const unordered_map<Activity*, enum_value*>& activityValueMap,
                const atomic_proposition& ap,
                fomula_automaton<>& foa) {
        enum_value nullValue("null");
        auto& var = *activityVarMap.at(pair(taskId, actId));
        auto& value = *activityValueMap.at(activity);
        auto& newValue = *activityValueMap.at(intent -> getActivity());
        auto topAP = (var == value);
        for (ID i = 1; i <= actId; i++) {
            auto& newVar = *activityVarMap.at(pair(taskId, i));
            add_transition(foa, newVar, nullValue, topAP & ap);
        }
        auto& newVar = *activityVarMap.at(pair(taskId, 0));
        add_transition(foa, newVar, newValue, topAP & ap);
    }

    void MutiTaskAnalyzer::analyzeReachability(AndroidStackMachine* a, int k) {
        a -> minimize();
        ID taskNum = a -> getAffinities().size();
        fomula_automaton<> foa;
        enum_value nullValue("null");
        vector<enum_value> activityValues({nullValue}),
                           actionValues({nullValue});
        unordered_map<pair<Activity*, Intent*>, enum_value*> actionValueMap;
        unordered_map<pair<ID, ID>, enum_variable*> activityVarMap;
        unordered_map<Activity*, enum_value*> activityValueMap;
        unordered_map<ID, enum_variable*> realActVarMap;
        vector<item*> items;
        ID id = 0;
        for (auto& [name, activity] : a -> getActivityMap()) {
            enum_value* v = new enum_value("a" + to_string(id++));
            activityValues.emplace_back(*v);
            activityValueMap[activity] = v;
            items.emplace_back(v);
        }
        for (ID i = 0; i < taskNum; i++) {
            for (ID j = 0; j < k; j++) {
                string name = "a_" + to_string(i) + "_" + to_string(j);
                enum_variable* v = new enum_variable(name, 
                                    activityValues.begin(), activityValues.end());
                add_control_state(foa, *v, (*v==nullValue));
                activityVarMap[pair(i,j)] = v;
            }
            string name = "r_" + to_string(i);
            enum_variable* v = new enum_variable(name, activityValues.begin(), activityValues.end());
            add_control_state(foa, *v, (*v==nullValue));
            realActVarMap[i] = v;
            items.emplace_back(v);
        }
        id = 0;
        for (auto& [activity, intents] : a -> getActionMap()) {
            for (auto intent : intents) {
                string name = "t" + to_string(id++);
                enum_value* v = new enum_value(name);
                actionValues.emplace_back(*v);
                actionValueMap[pair(activity, intent)] = v;
                items.emplace_back(v);
            }
        }
        enum_variable* actionVar = new enum_variable("t", actionValues.begin(), actionValues.end());
        items.emplace_back(actionVar);
        add_input_state(foa, *actionVar);
        //auto orderVar = add_control_state(foa)
        for (auto& [activity, intents] : a -> getActionMap()) {
            for (auto intent : intents) {
                auto actionValue = actionValueMap.at(pair(activity, intent));
                auto actionAtomic = (*actionVar == (*actionValue));
                for (ID i = 0; i < taskNum; i++) {
                    for (ID j = 0; j < k; j++) {
                        switch (AndroidStackMachine::getMode(intent)) {
                            case PUSH :
                                mkPUSH(activity, intent, i, j, k, activityVarMap,
                                       activityValueMap, actionAtomic, foa);
                                break;
                            case STOP :
                                mkSTOP(activity, intent, i, j, k, activityVarMap,
                                       activityValueMap, actionAtomic, foa);
                                break;
                            case CTOP :
                                mkCTOP(activity, intent, i, j, k, activityVarMap,
                                       activityValueMap, actionAtomic, foa);
                                break;
                            case CTSK :
                                mkCTSK(activity, intent, i, j, k, activityVarMap,
                                       activityValueMap, actionAtomic, foa);
                                break;
                            default :
                                break;
                        }
                    }
                }
            }
        }
        translate_nuxmv(foa, cout);
        for (auto v : items) {
            delete v;
            v = nullptr;
        }
    }
}

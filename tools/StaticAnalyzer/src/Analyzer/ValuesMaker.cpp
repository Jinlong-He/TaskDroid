#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {

    template<class T>
    void getOrders(size_t num, T null, vector<vector<T> >& orders) {
        vector<vector<T> > com;
        vector<T> vec, datas;
        for (size_t i = 0; i < num; i++) datas.emplace_back(i);
        for (size_t i = 1; i <= datas.size(); i++) {
            vec.clear();
            for (size_t j = 0; j < datas.size(); j++) {
                if (j < i) vec.emplace_back(datas[j]);
                else vec.emplace_back(null);
            }
            sort(vec.begin(), vec.end());
            com.emplace_back(vec);
        }
        for (size_t i = 0; i < com.size(); i++) {
            do {
                vector<T> n;
                for (size_t j = 0; j < com[i].size(); j++) {
                    n.emplace_back(com[i][j]);
                }
                orders.emplace_back(n);
            }
            while (next_permutation(com[i].begin(), com[i].end()));
        }
        orders.emplace_back(vector<T>(num, null));
    }

    enum_value* MultiTaskAnalyzer::getMainOrderValue() {
        vector<ID> mainOrder;
        for (auto i = 0; i < taskNum; i++) {
            if (i == mainTaskID) mainOrder.emplace_back(0);
            else mainOrder.emplace_back(-1);
        }
        return orderValueMap.at(mainOrder);
    }

    void MultiTaskAnalyzer::mkOrderValues() {
        if (taskNum == 1) return;
        getOrders(taskNum, (ID)-1, orders);
        for (auto& order : orders) {
            string name = "o";
            for (auto i : order) {
                if (i != -1) name += "_" + to_string(i);
                else name += "_null";
            }
            enum_value* v = new enum_value(name);
            orderValueMap[order] = v;
            orderValues.emplace_back(*v);
            items.emplace_back(v);
        }
    }

    void MultiTaskAnalyzer::mkActivityValues() {
        ID id = 0;
        for (auto& [name, activity] : a -> getActivityMap()) {
            enum_value* v = new enum_value("a" + to_string(id++));
            activityValues.emplace_back(*v);
            activityValueMap[activity] = v;
            items.emplace_back(v);
        }
        activityValueMap[nullptr] = &nullValue;
        mainActivityValue = *activityValueMap.at(a -> getMainActivity());
        mainTaskID = a -> getTaskID(a -> getMainActivity() -> getAffinity());
    }

    void MultiTaskAnalyzer::mkActionValues() {
        ID id = 0;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                string name = "t" + to_string(id++);
                enum_value* v = new enum_value(name);
                actionValues.emplace_back(*v);
                actionValueMap[pair(activity, intent)] = v;
                items.emplace_back(v);
                value2ActionMap[name] = pair(activity, pair(intent, finish));
            }
        }
    }

    void MultiTaskAnalyzer::mkVars() {
        for (ID i = 0; i < taskNum; i++) {
            ID kk = k;
            if (sitIDSet.count(i) > 0) kk = 1;
            for (ID j = 0; j < kk; j++) {
                string name = "a_" + to_string(i) + "_" + to_string(j);
                enum_variable* v = new enum_variable(name, 
                                    activityValues.begin(), activityValues.end());
                if (i == mainTaskID && j == 0) {
                    add_control_state(foa, *v, (*v==mainActivityValue));
                } else {
                    add_control_state(foa, *v, (*v==nullValue));
                }
                activityVarMap[pair(i,j)] = v;
                items.emplace_back(v);
            }
        }
        actionVar = enum_variable("t", actionValues.begin(), actionValues.end());
        add_input_state(foa, actionVar);
        if (taskNum > 1) {
            orderVar = enum_variable("o", orderValues.begin(), orderValues.end());
            add_control_state(foa, orderVar, orderVar == *getMainOrderValue());
        }
        mainTaskVar = bool_variable("mb");
        add_control_state(foa, mainTaskVar, mainTaskVar == bool_value(1));

        unordered_map<ID, Activities> realActivities;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                if (AndroidStackMachine::isNewMode(activity, intent) ||
                    AndroidStackMachine::getMode(activity, intent) == SIST ||
                    AndroidStackMachine::getMode(activity, intent) == MKTK) {
                    auto target = intent -> getActivity();
                    realActivities[getTaskID(target)].insert(target);
                }
            }
        }
        if (multiTaskIDMap.size() == 0 && a -> getAffinityMap().size() == 1) {
            isSingleTask = true;
            return;
        }
        for (auto& [taskID, mTaskID] : multiTaskIDMap)
            for (ID i = 0; i < m; i++)
                realActivities[mTaskID + i] = realActivities.at(taskID);
        auto mainActivity = a -> getMainActivity();
        realActivities[getTaskID(mainActivity)].insert(mainActivity);

        for (auto& [id, acts] : realActivities) {
            string name = "ra_" + to_string(id);
            vector<enum_value> realActivityValues({nullValue});
            for (auto act : acts)
                realActivityValues.emplace_back(*activityValueMap.at(act));
            enum_variable* v = new enum_variable(name, 
                                                 realActivityValues.begin(),
                                                 realActivityValues.end());
            realActivityVarMap[id] = v;
            items.emplace_back(v);
            if (id == mainTaskID)
                add_control_state(foa, *v, *v == mainActivityValue);
            else 
                add_control_state(foa, *v, *v == nullValue);
        }
    }
}



#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {
    void MultiTaskAnalyzer::getOutActivities() {
        auto activity = a -> getMainActivity();
        auto affinity = a -> getAffinityMap().at(activity -> getAffinity());
        unordered_map<ID, Activities> visited;
        visited[affinity].insert(activity);
        getOutActivities(affinity, activity, visited);
    }

    void MultiTaskAnalyzer::getOutActivities(ID taskID, Activity* activity,
                                             unordered_map<ID, Activities>& visited) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            auto newAffinity = a -> getAffinityMap().at(newActivity -> getAffinity());
            if (AndroidStackMachine::isNewMode(intent) && 
                taskID != newAffinity) {
                if (visited[newAffinity].insert(newActivity).second)
                    getOutActivities(newAffinity, newActivity, visited);
            } else {
                if (visited[taskID].insert(newActivity).second)
                    getOutActivities(taskID, newActivity, visited);
            }
        }
    }

    void MultiTaskAnalyzer::getAvailablePos() {
        auto activity = a -> getMainActivity();
        auto taskID = a -> getTaskID((activity -> getAffinity()));
        getOutActivities();
        availablePos[activity][taskID].insert(0);
        getAvailablePos(activity, taskID, 0);
    }

    void MultiTaskAnalyzer::getAvailablePos(Activity* activity, ID taskID, ID actID) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
            auto mode = AndroidStackMachine::getMode(intent);
            if (mode == PUSH ||
                mode == STOP && activity != newActivity ||
                mode == CTOP && activity != newActivity ||
                mode == PUSH_N && taskID == newTaskID ||
                mode == STOP_N && activity != newActivity && taskID == newTaskID ||
                mode == CTOP_N && activity != newActivity && taskID == newTaskID) {
                if (finish) {
                    if (availablePos[newActivity][taskID].insert(actID).second)
                        getAvailablePos(newActivity, taskID, actID);
                } else {
                    if (actID < k - 1) {
                        if (availablePos[newActivity][taskID].insert(actID + 1).second)
                            getAvailablePos(newActivity, taskID, actID + 1);
                    }
                }
            } else if (mode == CTSK) {
                if (availablePos[newActivity].insert(pair(taskID, 0)).second)
                    getAvailablePos(newActivity, taskID, 0);
            } else if (mode == PUSH_N && taskID != newTaskID ||
                       mode == STOP_N && taskID != newTaskID ||
                       mode == CTOP_N && taskID != newTaskID) {
                if (outActivities.count(newTaskID) == 0) {
                    if (availablePos[newActivity][newTaskID].insert(0).second)
                        getAvailablePos(newActivity, newTaskID, 0);
                } else {
                    unordered_set<ID> poses;
                    for (auto out : outActivities.at(newTaskID)) {
                        if (availablePos.count(out) == 0) continue;
                        for (auto& [tid, aids] : availablePos.at(out)) {
                            for (auto aid : aids)
                                if (tid == newTaskID) poses.insert(aid);
                        }
                    }
                    for (auto pos : poses) {
                        if (pos == k - 1) continue;
                        if (availablePos[newActivity][newTaskID].insert(pos + 1).second)
                            getAvailablePos(newActivity, newTaskID, pos + 1);
                    }
                }
            } else if (mode == CTSK_N && taskID != newTaskID) {
                if (availablePos[newActivity][newTaskID].insert(0).second)
                    getAvailablePos(newActivity, newTaskID, 0);
            }
        }
    }

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

    atomic_proposition MultiTaskAnalyzer::getTopOrderAP(ID taskID) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (order[taskID] == 0) ap = ap | orderVar == *v;
        }
        if (ap.to_string() == "FALSE") return atomic_proposition("TRUE");
        return ap;
    }

    atomic_proposition MultiTaskAnalyzer::getTopOrderAP(ID task0ID, ID task1ID) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (order[task0ID] == 0 && order[task1ID] == 1) 
                ap = ap | orderVar == *v;
        }
        if (ap.to_string() == "FALSE") return atomic_proposition("TRUE");
        return ap;
    }

    void MultiTaskAnalyzer::getNewOrder(const vector<ID>& order, ID newTaskID,
                                        vector<ID>& newOrder) {
        newOrder = order;
        newOrder[newTaskID] = 0;
        if (order[newTaskID] == -1) {
            for (ID i = 0; i < order.size(); i++) {
                if (order.at(i) != -1) newOrder[i] = order.at(i) + 1;
            }
        } else {
            auto o = order[newTaskID];
            for (ID i = 0; i < order.size(); i++) {
                if (order.at(i) < o) newOrder[i] = order.at(i) + 1;
            }
        }
    }

    void MultiTaskAnalyzer::getPopOrder(const vector<ID>& order,
                                        vector<ID>& newOrder) {
        newOrder = order;
        for (ID i = 0; i < order.size(); i++) {
            if (order.at(i) == -1) newOrder[i] = -1;
            else newOrder[i] = order.at(i) - 1;
        }
    }

    void MultiTaskAnalyzer::setActivity(ID taskID, ID actID, 
                                        Activity* activity,
                                        const atomic_proposition& ap) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        auto& value = *activityValueMap.at(activity);
        add_transition(foa, var, value, ap);
    }

    void MultiTaskAnalyzer::setFrontActivity(Activity* activity,
                                             const atomic_proposition& ap) {
        if (activity) {
            auto& value = *activityValueMap.at(activity);
            add_transition(foa, frontActivityVar, value, ap);
        }
    }

    void MultiTaskAnalyzer::setFrontActivity(ID taskID, ID actID,
                                             const atomic_proposition& ap) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        add_transition(foa, frontActivityVar, var, ap);
    }

    void MultiTaskAnalyzer::mkPUSH(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        if (!finish) {
            if (actID < k - 1)
                setActivity(taskID, actID + 1, newActivity, ap);
        } else {
            setActivity(taskID, actID, newActivity, ap);
        }
    }

    void MultiTaskAnalyzer::mkSTOP(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        if (!finish) {
            if (activity == newActivity) return;
            mkPUSH(activity, intent, finish, taskID, actID, ap);
        } else {
            if (activity == newActivity) {
                setActivity(taskID, actID, nullptr, ap);
            } else {
                setActivity(taskID, actID, newActivity, ap);
            }
        }
    }

    void MultiTaskAnalyzer::mkCTOP(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        if (!finish) {
            if (activity == newActivity) return;
        } else {
            if (activity ==  newActivity) {
                setActivity(taskID, actID, nullptr, ap);
            }
        }
        auto& newValue = *activityValueMap.at(newActivity);
        if (actID == 0) {
            auto& newVar = *activityVarMap.at(pair(taskID, actID));
            add_transition(foa, newVar, newValue, ap & (newVar == nullValue));
            return;
        }
        auto& var = *activityVarMap.at(pair(taskID, actID));
        auto& value = *activityValueMap.at(activity);
        auto topAP = atomic_proposition("TRUE");
        for (ID i = 0; i <= actID; i++) {
            auto& newVar = *activityVarMap.at(pair(taskID, i));
            auto p = (newVar == newValue);
            for (ID j = i + 1; j <= actID; j++) {
                p = p & (*activityVarMap.at(pair(taskID, j)) != newValue);
            }
            for (ID j = i + 1; j <= actID; j++) {
                auto& var = *activityVarMap.at(pair(taskID, j));
                add_transition(foa, var, nullValue, p & ap);
            }
            topAP = topAP & (newVar != newValue);
        }
        if (actID < k - 1) {
            auto& newVar = *activityVarMap.at(pair(taskID, actID + 1));
            add_transition(foa, newVar, newValue, topAP & ap);
        } 
    }

    void MultiTaskAnalyzer::mkCTSK(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto& newValue = *activityValueMap.at(intent -> getActivity());
        for (ID i = 1; i <= actID; i++) {
            auto& newVar = *activityVarMap.at(pair(taskID, i));
            add_transition(foa, newVar, nullValue, ap);
        }
        auto& newVar = *activityVarMap.at(pair(taskID, 0));
        add_transition(foa, newVar, newValue, ap);
    }

    void MultiTaskAnalyzer::switchTask(Intent* intent, ID taskID,
                                       const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
        if (taskID == newTaskID) return;
        for (const auto& order : orders) {
            if (order.at(taskID) == 0) {
                vector<ID> newOrder;
                getNewOrder(order, newTaskID, newOrder);
                auto& orderValue = *orderValueMap.at(order);
                auto& newOrderValue = *orderValueMap.at(newOrder);
                auto p = orderVar == orderValue & ap;
                add_transition(foa, orderVar, newOrderValue, p);
                if (isRealAct && order[newTaskID] == -1 && realActivityVarMap.count(newTaskID) > 0) {
                    auto& var = *realActivityVarMap.at(newTaskID);
                    auto& value = *activityValueMap.at(newActivity);
                    add_transition(foa, var, value, p);
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkPUSH_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
        auto& newValue = *activityValueMap.at(newActivity);
        auto realAP = atomic_proposition("TRUE");
        if (realActivityVarMap.count(newTaskID) > 0) {
            auto& realVar = *realActivityVarMap.at(newTaskID);
            realAP = (realVar != newValue);
        }
        if (newTaskID == taskID) {
            if (isRealAct)
                mkPUSH(activity, intent, finish, taskID, actID, realAP & ap);
            else 
                mkPUSH(activity, intent, finish, taskID, actID, ap);
        } else {
            for (ID j = 0; j < k; j++) {
                auto& newVar = *activityVarMap.at(pair(newTaskID, j));
                auto p = newVar == nullValue;
                if (j > 0) {
                    auto& var = *activityVarMap.at(pair(newTaskID, j - 1));
                    p = p & (var != nullValue);
                }
                add_transition(foa, newVar, newValue, realAP & p & ap);
            }
            if (finish) setActivity(taskID, actID, nullptr, ap);
        }
    }

    void MultiTaskAnalyzer::mkSTOP_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
        auto& newValue = *activityValueMap.at(newActivity);
        auto realAP = atomic_proposition("TRUE");
        if (realActivityVarMap.count(newTaskID) > 0) {
            auto& realVar = *realActivityVarMap.at(newTaskID);
            realAP = (realVar != newValue);
        }
        if (newTaskID == taskID) {
            if (isRealAct)
                mkSTOP(activity, intent, finish, taskID, actID, realAP & ap);
            else 
                mkSTOP(activity, intent, finish, taskID, actID, ap);
        } else {
            for (ID j = 0; j < k; j++) {
                auto& newVar = *activityVarMap.at(pair(newTaskID, j));
                auto p = newVar == nullValue;
                if (j > 0) {
                    auto& var = *activityVarMap.at(pair(newTaskID, j - 1));
                    p = p & (var != newValue) & (var != nullValue);
                }
                add_transition(foa, newVar, newValue, realAP & p & ap);
            }
            if (finish) setActivity(taskID, actID, nullptr, ap);
        }
    }

    void MultiTaskAnalyzer::mkCTOP_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
        if (newTaskID == taskID) {
            mkCTOP(activity, intent, finish, taskID, actID, ap);
        } else {
            for (ID i = 0; i < k; i++)
                mkCTOP(nullptr, intent, false, newTaskID, i, ap);
            if (finish) setActivity(taskID, actID, nullptr, ap);
        }
    }

    void MultiTaskAnalyzer::mkCTSK_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const atomic_proposition& ap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = a -> getTaskID(newActivity -> getAffinity());
        if (newTaskID == taskID) {
            mkCTSK(activity, intent, finish, taskID, actID, ap);
        } else {
            mkCTSK(nullptr, intent, false, newTaskID, k - 1, ap);
            if (finish) setActivity(taskID, actID, nullptr, ap);
        }
    }

    void MultiTaskAnalyzer::mkPOP(ID taskID, ID actID, 
                                  const atomic_proposition& ap) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        auto p = var != nullValue;
        if (actID == k - 1) {
            add_transition(foa, var, nullValue, p & ap);
        } else {
            auto& newVar = *activityVarMap.at(pair(taskID, actID + 1));
            add_transition(foa, var, nullValue, ap & p & newVar == nullValue);
            if (actID == 0 && taskNum > 1) {
                for (const auto& order : orders) {
                    if (order.at(taskID) == 0) {
                        auto& orderValue = *orderValueMap.at(order);
                        vector<ID> newOrder;
                        getPopOrder(order, newOrder);
                        auto& newOrderValue = *orderValueMap.at(newOrder);
                        add_transition(foa, orderVar, newOrderValue, 
                                       orderVar == orderValue &
                                       p & newVar == nullValue & ap);
                    }
                }
            }
        }
    }

    void MultiTaskAnalyzer::loadASM(AndroidStackMachine* a) {
        this -> a = a;
        this -> a -> fomalize();
        taskNum = this -> a -> getAffinityMap().size();
        bool realActFlag = false;
        for (auto& [activity, actions] : a -> getActionMap())
            for (auto& [intent, finish]  : actions)
                if (AndroidStackMachine::getMode(intent) == PUSH_N ||
                    AndroidStackMachine::getMode(intent) == STOP_N)
                    realActFlag = true;
        if (taskNum > 1 || realActFlag)
            isRealAct = true;
        mkVarsValues();
    }

    void MultiTaskAnalyzer::clear() {
        foa = fomula_automaton<>();
        actionValues.clear();
        activityValues.clear();
        orders.clear();
        orderValues = vector<enum_value>({nullValue});
        actionValues = vector<enum_value>({nullValue, popValue});
        activityValues = vector<enum_value>({nullValue});
        orderValueMap.clear();
        activityValueMap.clear();
        activityBVarMap.clear();
        actionBVarMap.clear();
        activityVarMap.clear();
        actionValueMap.clear();
        value2ActionMap.clear();
        availablePos.clear();
        outActivities.clear();
        isMkVarsValues = false;
        isTranslate2Foa = false;
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
                value2ActionMap[name] = pair(activity, intent);
            }
        }
    }

    enum_value* MultiTaskAnalyzer::getMainOrderValue() {
        vector<ID> mainOrder;
        for (auto i = 0; i < taskNum; i++) {
            if (i == mainTaskID) mainOrder.emplace_back(0);
            else mainOrder.emplace_back(-1);
        }
        return orderValueMap.at(mainOrder);
    }

    void MultiTaskAnalyzer::mkActivityVars() {
        for (ID i = 0; i < taskNum; i++) {
            for (ID j = 0; j < k; j++) {
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
        if (taskNum == 1) {
            //frontActivityVar = enum_variable("fa", activityValues.begin(), 
            //                                       activityValues.end());
            //add_control_state(foa, frontActivityVar, 
            //                  frontActivityVar == mainActivityValue);
        } else {
            orderVar = enum_variable("o", orderValues.begin(), orderValues.end());
            add_control_state(foa, orderVar, orderVar == *getMainOrderValue());
        }
    }

    void MultiTaskAnalyzer::mkRealActivityVars() {
        unordered_map<string, Activities> realActivities;
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                if (AndroidStackMachine::isNewMode(intent)) {
                    auto target = intent -> getActivity();
                    realActivities[target -> getAffinity()].insert(target);
                }
            }
        }
        for (auto& [affinity, acts] : realActivities) {
            ID id = a -> getTaskID(affinity);
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

    void MultiTaskAnalyzer::mkVarsValues() {
        if (isMkVarsValues) return;
        isMkVarsValues = true;
        mkOrderValues();
        mkActivityValues();
        mkActionValues();
        mkActivityVars();
        getAvailablePos();
        if (isRealAct) mkRealActivityVars();
    }

    void MultiTaskAnalyzer::translate2FOA() {
        if (isTranslate2Foa) return;
        isTranslate2Foa = true;
        mkVarsValues();
        for (ID i = 0; i < taskNum; i++) {
            auto orderAP = getTopOrderAP(i);
            if (taskNum == 1) orderAP = atomic_proposition("TRUE");
            for (ID j = 0; j < k; j++) {
                mkPOP(i, j, orderAP & actionVar == popValue);
            }
        }
        for (auto& [activity, actions] : a -> getActionMap()) {
            auto& value = *activityValueMap.at(activity);
            for (auto& [intent, finish] : actions) {
                auto& actionValue = *actionValueMap.at(pair(activity, intent));
                auto actionAP = (actionVar == actionValue);
                auto newTaskID = a -> getTaskID(intent -> getActivity() -> getAffinity());
                if (availablePos.count(activity) == 0) continue;
                for (auto& [i, js] : availablePos.at(activity)) {
                    auto orderAP = getTopOrderAP(i);
                    if (taskNum == 1) orderAP = atomic_proposition("TRUE");
                    for (auto j : js) {
                        auto& var = *activityVarMap.at(pair(i, j));
                        auto ap = (var == value) & orderAP & actionAP;
                        auto ap_N = (var == value) & actionAP;
                        if (j + 1 < k) {
                            auto& var = *activityVarMap.at(pair(i, j + 1));
                            ap = (var == nullValue) & ap;
                            ap_N = (var == nullValue) & ap_N;
                        }
                        if (actionBVarMap.count(pair(activity, intent)) > 0) {
                            auto& var = *actionBVarMap.at(pair(activity, intent));
                            add_transition(foa, var, bool_value(1), ap);
                        }
                        switch (AndroidStackMachine::getMode(intent)) {
                            case PUSH :
                                mkPUSH(activity, intent, finish, i, j, ap);
                                break;
                            case STOP :
                                mkSTOP(activity, intent, finish, i, j, ap);
                                break;
                            case CTOP :
                                mkCTOP(activity, intent, finish, i, j, ap);
                                break;
                            case CTSK :
                                mkCTSK(activity, intent, finish, i, j, ap);
                                break;
                            case RTOF :
                                mkPUSH(activity, intent, finish, i, j, ap);
                                break;
                            case PUSH_N :
                                switchTask(intent, i, ap_N);
                                mkPUSH_N(activity, intent, finish, i, j, ap);
                                break;
                            case STOP_N :
                                break;
                            case CTOP_N :
                                switchTask(intent, i, ap_N);
                                mkCTOP_N(activity, intent, finish, i, j, ap);
                                break;
                            case CTSK_N :
                                switchTask(intent, i, ap_N);
                                mkCTSK_N(activity, intent, finish, i, j, ap);
                                break;
                            case RTOF_N :
                                mkPUSH_N(activity, intent, finish, i, j, ap);
                                break;
                            default :
                                break;
                        }
                    }
                }
            }
        }
        for (auto& [p, var] : activityVarMap) {
            add_transition(foa, *var, *var, atomic_proposition("TRUE"));
        }
        if (taskNum > 1) 
            add_transition(foa, orderVar, orderVar, atomic_proposition("TRUE"));
        if (actionBVarMap.size() > 0) {
            for (auto& [pair, var] : actionBVarMap) {
                add_transition(foa, *var, *var, atomic_proposition("TRUE"));
            }
        }
        if (isRealAct) {
            for (auto& [p, var] : realActivityVarMap) {
                add_transition(foa, *var, *var, atomic_proposition("TRUE"));
            }
        }
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
        os << "-Acitivity Transition Coverage: " << 1.0 * values.size()/value2ActionMap.size() << endl;;
        os << "-Acitivity Transition Coverage: " << values.size() << "/" << value2ActionMap.size() << endl;;
        return true;
    }
}


#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {
    
    void MultiTaskAnalyzer::mkVarsValues() {
        if (isMkVarsValues) return;
        isMkVarsValues = true;
        mkOrderValues();
        mkActivityValues();
        mkActionValues();
        mkVars();
        mkLegalPos();
    }

    atomic_proposition MultiTaskAnalyzer::mkIsTopNullAP(ID taskID, ID actID) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        if (actID == 0) {
            return (var == nullValue);
        } else {
            if (sitIDSet.count(taskID) > 0) {
                return atomic_proposition("FALSE");
            } else {
                auto& actVar = *activityVarMap.at(pair(taskID, actID - 1));
                return (var == nullValue & actVar != nullValue);
            }
        }
    }

    atomic_proposition MultiTaskAnalyzer::mkIsTopNonNullAP(ID taskID, ID actID) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        if (actID == k - 1) {
            return (var != nullValue);
        } else {
            if (sitIDSet.count(taskID) > 0) {
                return atomic_proposition("FALSE");
            } else {
                auto& actVar = *activityVarMap.at(pair(taskID, actID + 1));
                return (var != nullValue & actVar == nullValue);
            }
        }
    }

    atomic_proposition MultiTaskAnalyzer::mkIsTopActAP(ID taskID, ID actID,
                                                    Activity* activity) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        auto& value = *activityValueMap.at(activity);
        if (sitIDSet.count(taskID) > 0) {
            if (actID == 0) {
                return (var == value);
            } else {
                return atomic_proposition("FALSE");
            }
        } else {
            if (actID == k - 1) {
                return (var == value);
            } else {
                auto& nullVar = *activityVarMap.at(pair(taskID, actID + 1));
                return (var == value & nullVar == nullValue);
            }
        }
    }

    atomic_proposition MultiTaskAnalyzer::mkIsTopTaskAP(ID taskID) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (order[taskID] == 0) ap = ap | orderVar == *v;
        }
        if (taskNum > 1) return ap;
        return atomic_proposition("TRUE");
    }

    atomic_proposition MultiTaskAnalyzer::mkIsTopTaskAP(ID taskID1, ID taskID2) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (order[taskID1] == 0 && order[taskID2] != -1) ap = ap | orderVar == *v;
        }
        if (taskNum > 1) return ap;
        return atomic_proposition("TRUE");
    }

    void MultiTaskAnalyzer::mkADDA(ID taskID, ID actID, Activity* activity,
                                   const atomic_proposition& ap) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        auto& value = *activityValueMap.at(activity);
        add_transition(foa, var, value, ap);
    }

    void MultiTaskAnalyzer::mkDELA(ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto& var = *activityVarMap.at(pair(taskID, actID));
        add_transition(foa, var, nullValue, ap);
    }

    void MultiTaskAnalyzer::mkPOPT(ID taskID, 
                                   const atomic_proposition& ap) {
        vector<Order> topOrders;
        getTopOrders(taskID, topOrders);
        for (auto& order : topOrders) {
            Order popOrder;
            getPopOrder(order, popOrder);
            auto& value = *orderValueMap.at(order);
            auto& newValue = getPopOrderValue(order);
            add_transition(foa, orderVar, newValue, (orderVar == value) & ap);
        }
    }

    void MultiTaskAnalyzer::mkPUSH(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        if (!finish) {
            if (actID < k - 1)
                mkADDA(taskID, actID + 1, newActivity, orderAP & ap);
        } else {
            mkADDA(taskID, actID, newActivity, orderAP & ap);
        }
    }

    void MultiTaskAnalyzer::mkSTOP(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        if (!finish) {
            if (activity == newActivity) return;
            mkPUSH(activity, intent, finish, taskID, actID, ap);
        } else {
            if (activity == newActivity) {
                mkDELA(taskID, actID, orderAP & ap);
                if (actID == 0) mkPOPT(taskID, ap);
            } else {
                mkADDA(taskID, actID, newActivity, orderAP & ap);
            }
        }
    }

    void MultiTaskAnalyzer::mkCTOP(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        if (activity == newActivity) {
            if (finish) {
                mkDELA(taskID, actID, orderAP & ap);
                if (actID == 0) mkPOPT(taskID, ap);
            } 
            else return;
        } else {
            if (legalPos.count(newActivity) == 0 ||
                legalPos.at(newActivity).count(taskID) == 0) return;
            auto& set = legalPos.at(newActivity).at(taskID);
            vector<ID> poses(set.begin(), set.end());
            auto noAP = atomic_proposition("TRUE");
            for (ID i = poses.size() - 1; i != -1; i--) {
                if (poses.at(i) >= actID) continue;
                auto& var = *activityVarMap.at(pair(taskID, poses.at(i)));
                auto p = (var == newValue);
                for (ID j = poses.at(i) + 1; j <= actID; j++)
                    mkDELA(taskID, j, orderAP & noAP & p & ap);
                noAP = noAP & (var != newValue);
            }
            if (!finish && actID < k - 1)
                mkADDA(taskID, actID + 1, newActivity, orderAP & noAP & ap);
            if (finish) 
                mkADDA(taskID, actID, newActivity, orderAP & noAP & ap);
        }
    }

    void MultiTaskAnalyzer::mkRTOF(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        if (activity == newActivity) {
            if (finish) {
                mkDELA(taskID, actID, orderAP & ap);
                if (actID == 0) mkPOPT(taskID, ap);
            } 
            else return;
        } else {
            if (legalPos.count(newActivity) == 0 ||
                legalPos.at(newActivity).count(taskID) == 0) return;
            auto& set = legalPos.at(newActivity).at(taskID);
            vector<ID> poses(set.begin(), set.end());
            auto noAP = atomic_proposition("TRUE");
            for (ID i = poses.size() - 1; i != -1; i--) {
                if (poses.at(i) >= actID) continue;
                auto& var = *activityVarMap.at(pair(taskID, poses.at(i)));
                auto p = (var == newValue);
                mkADDA(taskID, actID, newActivity, noAP & p & ap);
                mkADDA(taskID, poses.at(i), activity, noAP & p & ap);
                noAP = noAP & (var != newValue);
            }
            if (!finish && actID < k - 1)
                mkADDA(taskID, actID + 1, newActivity, orderAP & noAP & ap);
            if (finish) 
                mkADDA(taskID, actID, newActivity, orderAP & noAP & ap);
        }
    }

    void MultiTaskAnalyzer::mkCTSK(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        for (ID j = 0; j <= actID; j++) {
            mkDELA(taskID, j, orderAP & ap);
        }
    }

    void MultiTaskAnalyzer::mkSIST(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        auto orderAP = mkIsTopTaskAP(taskID);
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        auto newTaskID = getTaskID(newActivity);
        vector<Order> topOrders;
        getTopOrders(taskID, topOrders);
        bool f = actID == 0 && finish;
        for (auto& order : topOrders) {
            auto& value = *orderValueMap.at(order);
            auto orderAP = (orderVar == value);
            auto& realVar = *realActivityVarMap.at(newTaskID);
            auto& orderValue = getSwtOrderValue(order, newTaskID, f);
            add_transition(foa, orderVar, orderValue, orderAP & ap);
            add_transition(foa, realVar, newValue, orderAP & ap);
            mkADDA(newTaskID, 0, newActivity, orderAP & ap);
        }
    }

    void MultiTaskAnalyzer::mkSWTK(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap,
                                   TaskPropMap& taskPropMap) {
        auto newActivity = intent -> getActivity();
        auto newTaskID = getTaskID(newActivity);
        auto& newValue = *activityValueMap.at(newActivity);
        if (isSingleTask) {
            taskPropMap[Order()][taskID].emplace_back(pair(0, ap));
            return;
        }
        vector<Order> topOrders;
        getTopOrders(taskID, topOrders);
        bool f = actID == 0 && finish;
        for (auto& order : topOrders) {
            auto& value = *orderValueMap.at(order);
            auto orderAP = (orderVar == value);
            auto tasks = getTasks(order, newTaskID);
            if (tasks.size() == 0) {
                auto& realVar = *realActivityVarMap.at(newTaskID);
                auto& orderValue = getSwtOrderValue(order, newTaskID, f);
                add_transition(foa, orderVar, orderValue, orderAP & ap);
                add_transition(foa, realVar, newValue, orderAP & ap);
                mkADDA(newTaskID, 0, newActivity, orderAP & ap);
            } else {
                atomic_proposition noRealAP = atomic_proposition("TRUE");
                for (ID i = 0; i < tasks.size(); i++) {
                    auto& realVar = *realActivityVarMap.at(tasks.at(i));
                    auto realAP = (realVar == newValue);
                    noRealAP = noRealAP & (realVar != newValue);
                    auto& orderValue = getSwtOrderValue(order, tasks.at(i), f);
                    for (ID j = 0; j < i; j++) {
                        auto& realVar = *realActivityVarMap.at(tasks.at(j));
                        realAP = realAP & (realVar != newValue);
                    }
                    auto p = orderAP & ap & realAP;
                    taskPropMap[order][tasks.at(i)].emplace_back(pair(1, p));
                    add_transition(foa, orderVar, orderValue, p);
                }
                auto p = orderAP & ap & noRealAP;
                auto& orderValue = getSwtOrderValue(order, tasks.at(0), f);
                taskPropMap[order][tasks.at(0)].emplace_back(pair(0, p));
                add_transition(foa, orderVar, orderValue, p);
            }
        }
    }

    void MultiTaskAnalyzer::mkPUSH_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const TaskPropMap& taskPropMap) {
        auto newActivity = intent -> getActivity();
        for (auto& [order, map] : taskPropMap) {
            for (auto& [newTaskID, vec] : map) {
                for (auto& [real, prop] : vec) {
                    if (newTaskID == taskID) {
                        if ((!real || real && 
                            newActivity == a -> getMainActivity())) {
                            if (!finish && actID < k - 1)
                                mkADDA(taskID, actID + 1, newActivity, prop);
                            if (finish) mkADDA(taskID, actID, newActivity, prop);
                        } else {
                            if (finish) mkDELA(taskID, actID, prop);
                        }
                    } else {
                        if (finish) mkDELA(taskID, actID, prop);
                        if ((!real || real && 
                            newActivity == a -> getMainActivity()) && 
                            legalPos.count(newActivity) > 0 &&
                            legalPos.at(newActivity).count(newTaskID) > 0) {
                            auto& poses = legalPos.at(newActivity).at(newTaskID);
                            for (auto j : poses) {
                                if (j == 0) continue;
                                auto newTopAP = mkIsTopNullAP(newTaskID, j);
                                mkADDA(newTaskID, j, newActivity, prop & newTopAP);
                            }
                        }
                    }
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkSTOP_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const TaskPropMap& taskPropMap) {
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        for (auto& [order, map] : taskPropMap) {
            for (auto& [newTaskID, vec] : map) {
                for (auto& [real, prop] : vec) {
                    if (newTaskID == taskID) {
                        if ((!real || real && 
                            newActivity == a -> getMainActivity()) &&
                            newActivity != activity) {
                            if (!finish && actID < k - 1)
                                mkADDA(taskID, actID + 1, newActivity, prop);
                            if (finish) mkADDA(taskID, actID, newActivity, prop);
                        } else {
                            if (finish) mkDELA(taskID, actID, prop);
                        }
                    } else {
                        if (finish) mkDELA(taskID, actID, prop);
                        if ((!real || real && 
                            newActivity == a -> getMainActivity()) && 
                            legalPos.count(newActivity) > 0 &&
                            legalPos.at(newActivity).count(newTaskID) > 0) {
                            auto& poses = legalPos.at(newActivity).at(newTaskID);
                            for (auto j : poses) {
                                if (j == 0) continue;
                                auto& var = *activityVarMap.at(pair(newTaskID, j - 1));
                                auto p = (var != newValue);
                                auto newTopAP = mkIsTopNullAP(newTaskID, j);
                                mkADDA(newTaskID, j, newActivity, prop & newTopAP & p);
                            }
                        }
                    }
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkCTOP_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const TaskPropMap& taskPropMap) {
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        for (auto& [order, map] : taskPropMap) {
            for (auto& [newTaskID, vec] : map) {
                for (auto& [real, prop] : vec) {
                    if (newTaskID == taskID) {
                        if (activity == newActivity && finish) 
                            mkDELA(taskID, actID, prop);
                        if (activity != newActivity) {
                            if (legalPos.count(newActivity) == 0 ||
                                legalPos.at(newActivity).count(taskID) == 0) return;
                            auto& set = legalPos.at(newActivity).at(taskID);
                            vector<ID> poses(set.begin(), set.end());
                            auto noAP = atomic_proposition("TRUE");
                            for (ID i = poses.size() - 1; i != -1; i--) {
                                if (poses.at(i) >= actID) continue;
                                auto& var = *activityVarMap.at(pair(taskID, poses.at(i)));
                                auto p = (var == newValue);
                                for (ID j = poses.at(i) + 1; j <= actID; j++)
                                    mkDELA(taskID, j, noAP & p & prop);
                                noAP = noAP & (var != newValue);
                            }
                            if (!finish && actID < k - 1)
                                mkADDA(taskID, actID + 1, newActivity, noAP & prop);
                            if (finish) 
                                mkADDA(taskID, actID, newActivity, noAP & prop);
                        } 
                    } else {
                        if (finish) mkDELA(taskID, actID, prop);
                        if (legalPos.count(newActivity) == 0 ||
                            legalPos.at(newActivity).count(newTaskID) == 0) return;
                        auto& set = legalPos.at(newActivity).at(newTaskID);
                        vector<ID> poses(set.begin(), set.end());
                        for (ID actID = 1; actID < k; actID++) {
                            auto topAP = mkIsTopNullAP(newTaskID, actID);
                            auto noAP = atomic_proposition("TRUE");
                            for (ID i = poses.size() - 1; i != -1; i--) {
                                if (poses.at(i) >= actID) continue;
                                auto& var = *activityVarMap.at(pair(newTaskID, poses.at(i)));
                                auto p = (var == newValue);
                                for (ID j = poses.at(i) + 1; j <= actID; j++)
                                    mkDELA(newTaskID, j, noAP & p & topAP & prop);
                                noAP = noAP & (var != newValue);
                            }
                                mkADDA(newTaskID, actID, newActivity, noAP & topAP & prop);
                        }
                    }
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkCTSK_N(Activity* activity, Intent* intent, 
                                     bool finish, ID taskID, ID actID,
                                     const TaskPropMap& taskPropMap) {
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        for (auto& [order, map] : taskPropMap) {
            for (auto& [newTaskID, vec] : map) {
                for (auto& [real, prop] : vec) {
                    if (newTaskID == taskID) {
                        for (ID j = 1; j <= actID; j++) {
                            mkDELA(taskID, actID, prop);
                        }
                        mkADDA(taskID, 0, newActivity, prop);
                    } else {
                        for (ID j = 1; j < k; j++) {
                            auto p = mkIsTopNonNullAP(newTaskID, j);
                            for (ID jj = 1; jj <= j; jj++)
                                mkDELA(newTaskID, jj, p & prop);
                        }
                        mkADDA(newTaskID, 0, newActivity, prop);
                    }
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkMKTK(Activity* activity, Intent* intent, 
                                   bool finish, ID taskID, ID actID,
                                   const atomic_proposition& ap) {
        vector<Order> topOrders;
        getTopOrders(taskID, topOrders);
        auto newActivity = intent -> getActivity();
        auto& newValue = *activityValueMap.at(newActivity);
        bool f = finish && actID == 0;
        for (auto& order : topOrders) {
            auto& value = *orderValueMap.at(order);
            auto orderAP = (orderVar == value);
            for (ID i = 0; i < m; i++) {
                ID newTaskID = multiTaskIDMap.at(getTaskID(newActivity)) + i;
                if (order[newTaskID] == -1) {
                    auto& newOrderValue = getSwtOrderValue(order, newTaskID, f);
                    auto& realVar = *realActivityVarMap.at(newTaskID);
                    add_transition(foa, orderVar, newOrderValue, orderAP & ap);
                    add_transition(foa, realVar, newValue, orderAP & ap);
                    mkADDA(newTaskID, 0, newActivity, orderAP & ap);
                    if (finish) {
                        mkDELA(taskID, actID, orderAP & ap);
                    }
                    break;
                }
            }
        }
    }

    void MultiTaskAnalyzer::mkPOP() {
        for (ID i = 0; i < taskNum; i++) {
            auto orderAP = mkIsTopTaskAP(i);
            auto ap = actionVar == popValue & orderAP;
            if (taskNum == 1) orderAP = atomic_proposition("TRUE");
            for (ID j = 0; j < k; j++) {
                if (activityVarMap.count(pair(i,j)) == 0) break;
                auto& var = *activityVarMap.at(pair(i,j));
                auto actAP = mkIsTopNonNullAP(i,j);
                mkDELA(i, j, ap & actAP);
                if (j == 0) mkPOPT(i, (actionVar == popValue) & actAP);
                if (i == mainTaskID && j == 0) 
                    add_transition(foa, mainTaskVar, bool_value(0), ap & actAP);
            }
        }
    }

    void MultiTaskAnalyzer::translate2FOA() {
        if (isTranslate2Foa) return;
        isTranslate2Foa = true;
        mkVarsValues();
        mkPOP();
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish] : actions) {
                auto& actionValue = *actionValueMap.at(pair(activity, intent));
                auto actionAP = (actionVar == actionValue);
                if (legalPos.count(activity) == 0) continue;
                for (auto& [i, js] : legalPos.at(activity)) {
                    for (auto j : js) {
                        auto ap = mkIsTopActAP(i, j, activity) & actionAP;
                        TaskPropMap taskPropMap;
                        switch (AndroidStackMachine::getMode(activity, intent)) {
                            case SIST :
                                mkSIST(activity, intent, finish, i, j, ap);
                                break;
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
                                mkRTOF(activity, intent, finish, i, j, ap);
                                break;
                            case MKTK :
                                mkMKTK(activity, intent, finish, i, j, ap);
                                break;
                            case PUSH_N :
                                mkSWTK(activity, intent, finish, i, j, ap, taskPropMap);
                                mkPUSH_N(activity, intent, finish, i, j, taskPropMap);
                                break;
                            case STOP_N :
                                mkSWTK(activity, intent, finish, i, j, ap, taskPropMap);
                                mkSTOP_N(activity, intent, finish, i, j, taskPropMap);
                                break;
                            case CTOP_N :
                                mkSWTK(activity, intent, finish, i, j, ap, taskPropMap);
                                mkCTOP_N(activity, intent, finish, i, j, taskPropMap);
                                break;
                            case CTSK_N :
                                mkSWTK(activity, intent, finish, i, j, ap, taskPropMap);
                                mkCTSK_N(activity, intent, finish, i, j, taskPropMap);
                                break;
                            case RTOF_N :
                                mkSWTK(activity, intent, finish, i, j, ap, taskPropMap);
                                mkPUSH_N(activity, intent, finish, i, j, taskPropMap);
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
        for (auto& [p, var] : realActivityVarMap) {
            add_transition(foa, *var, *var, atomic_proposition("TRUE"));
        }
        add_transition(foa, mainTaskVar, mainTaskVar, atomic_proposition("TRUE"));
    }

}

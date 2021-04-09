
#include <iostream>
#include <queue>
#include <cmath>
#include "Analyzer/FragmentAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
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

    template<class T1, class T2>
    void getOrders(const vector<T1>& orders, const vector<T2>& vec,
                   vector<vector<pair<T1, T2> > >& newOrders) {
        ID p = orders.size(), k = vec.size();
        newOrders = 
            vector<vector<pair<T1, T2> > >(pow(k, p), vector<pair<T1, T2> >(p));
        for (ID l = 0; l < newOrders.size(); l++) {
            for (ID i = 0; i < p; i++) {
                ID n = ((int)(l/(pow(k,p-i-1))))%(k);
                newOrders[l][i] = pair(orders[i], vec[n]);
            }
        }

    }

    void FragmentAnalyzer::mkFragmentValues() {
        ID i = 0, j = 0;
        for (auto& [name, fragment] : a -> getFragmentMap()) {
            for (auto& [f, transactions] : a -> getFragmentTransactionMap()) {
                for (auto transaction : transactions) {
                    auto v = new enum_value("bf" + to_string(i) + "_" + to_string(j++));
                    backTransactionValues.emplace_back(*v);
                    backFragmentValueMap[pair(fragment, transaction)] = v;
                    items.emplace_back(v);
                }
            }
            enum_value* v = new enum_value("f" + to_string(i++));
            fragmentValues.emplace_back(*v);
            fragmentValueMap[fragment] = v;
            items.emplace_back(v);
        }
        fragmentValueMap[nullptr] = &nullValue;
    }

    void FragmentAnalyzer::mkTransactionValues() {
        ID id = 0;
        for (auto& [fragment, transactions] : a -> getFragmentTransactionMap()) {
            for (auto& transaction : transactions) {
                string name = "t" + to_string(id++);
                enum_value* v = new enum_value(name);
                transactionValues.emplace_back(*v);
                transactionValueMap[transaction] = v;
                items.emplace_back(v);
                if (transaction -> isAddTobackStack()) {
                    backTransactionValues.emplace_back(*v);
                }
            }
        }
    }

    void FragmentAnalyzer::mkOrderValues() {
        vector<vector<ID> > oldOrders;
        getOrders(k, (ID)-1, oldOrders);
        vector<pair<enum_value*, ID> > fts;
        for (auto ft : backTransactionValues) {
            for (ID i = 0; i < h; i++) fts.emplace_back(pair(&ft, i));
            fts.emplace_back(pair(&ft, -1));
        }
        for (auto& order : oldOrders) {
            vector<vector<pair<ID, pair<enum_value*, ID> > > > newOrders;
            getOrders(order, fts, newOrders);
            for (auto& newOrder : newOrders) {
                bool flag = true;
                for (auto& [i, pair] : newOrder) {
                    if (i == -1 && pair.second != -1) flag = false;
                }
                if (flag) orders.emplace_back(newOrder);
            }
        }
        for (auto& order : orders) {
            string name = "o";
            for (auto& [i, pair] : order) {
                if (pair.second == -1) name += "_S";
                else name += "_H" + to_string(pair.second) + pair.first -> identifier() + "o";
                if (i != -1) name += to_string(i);
                else name += "null";
            }
            enum_value* v = new enum_value(name);
            orderValueMap[order] = v;
            orderValues.emplace_back(*v);
            items.emplace_back(v);
        }
    }

    void FragmentAnalyzer::mkOrderVars() {
        for (ID i = 0; i < viewNum; i++) {
            auto v = new enum_variable("o" + to_string(i), orderValues.begin(),
                                                           orderValues.end());
            add_control_state(foa, *v, *v==nullValue);
            items.emplace_back(v);
        }
    }

    void FragmentAnalyzer::mkTransactionVars() {
        for (ID j = 0; j < k; j++) {
            string name = "f_" + to_string(j);
            auto v = new enum_variable(name, backTransactionValues.begin(), 
                                             backTransactionValues.end());
            add_control_state(foa, *v, (*v==nullValue));
            backTransactionVarMap[j] = v;
            items.emplace_back(v);
        }
        transactionVar = enum_variable("t", transactionValues.begin(), 
                                            transactionValues.end());
        add_input_state(foa, transactionVar);
    }

    void FragmentAnalyzer::mkFragmentVars() {
        ID id = 0;
        for (ID i = 0; i < viewNum; i++) {
            auto& map = fragmentVarMap[i];
            for (ID l = 0; l < h; l++) {
                for (ID j = 0; j < k; j++) {
                    string name = "f_" + to_string(i) + "_" + to_string(l) + "_" + to_string(j);
                    auto v = new enum_variable(name, fragmentValues.begin(),
                                                     fragmentValues.end());
                    if (j == 0) {
                        add_control_state(foa, *v, (*v==nullValue));
                    } else {
                        add_control_state(foa, *v, (*v==nullValue));
                    }
                    map[pair(l,j)] = v;
                    items.emplace_back(v);
                }
            }
        }
    }

    void FragmentAnalyzer::getTopOrderAP(ID viewID, ID stackID,
                                         atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            ID min = order[stackID].first;
            if (min == -1) continue;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.second == -1) {
                    min = min < order[i].first ? min : order[i].first;
                }
            }
            if (min == order[stackID].first) {
                auto& value = *orderValueMap.at(order);
                ap = ap | (var == value);
            }
        }
    }

    //void FragmentAnalyzer::mkREP_B(FragmentAction* action,
    //                               FragmentTransaction* transaction,
    //                               const atomic_proposition& ap) {
    //    auto viewID = a -> getViewID(action -> getViewID());
    //    auto fragment = action -> getFragment();
    //    auto& newValue = transaction -> isAddTobackStack() ?
    //                     *backFragmentValueMap.at(pair(fragment, transaction)) :
    //                     *fragmentValueMap.at(fragment);
    //    for (ID j = k - 1; j >= 0; j--) {
    //        auto& var = *fragmentVarMap.at(pair(viewID, j));
    //        for (auto& [name, f] : a -> getFragmentMap()) {
    //            auto& value = *fragmentValueMap.at(f);
    //            auto p = var == value;
    //        }
    //        add_transition(foa, var, nullValue, ap);
    //    }
    //    auto& newVar = *fragmentVarMap.at(pair(viewID, 0));
    //    add_transition(foa, newVar, newValue, ap);
    //}

    void FragmentAnalyzer::mkREP(FragmentAction* action,
                                 ID stackID,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        auto viewID = a -> getViewID(action -> getViewID());
        auto fragment = action -> getFragment();
        auto& newValue = transaction -> isAddTobackStack() ?
                         *backFragmentValueMap.at(pair(fragment, transaction)) :
                         *fragmentValueMap.at(fragment);
        const auto& map = fragmentVarMap.at(stackID);
        for (ID j = 1; j < k; j++) {
            auto& var = *map.at(pair(viewID, j));
            add_transition(foa, var, nullValue, ap);
        }
        auto& newVar = *map.at(pair(viewID, 0));
        add_transition(foa, newVar, newValue, ap);
    }

    //void FragmentAnalyzer::mkADD(FragmentAction* action,
    //                             FragmentTransaction* transaction,
    //                             const atomic_proposition& ap) {
    //    auto viewID = a -> getViewID(action -> getViewID());
    //    auto fragment = action -> getFragment();
    //    auto& newValue = transaction -> isAddTobackStack() ?
    //                     *backFragmentValueMap.at(pair(fragment, transaction)) :
    //                     *fragmentValueMap.at(fragment);
    //    for (ID j = 0; j < k; j++) {
    //        auto& newVar = *fragmentVarMap.at(pair(viewID, j));
    //        auto p = newVar == nullValue;
    //        if (j > 0) {
    //            auto& var = *fragmentVarMap.at(pair(viewID, j - 1));
    //            p = p & var != nullValue;
    //        }
    //        add_transition(foa, newVar, newValue, p & ap);
    //    }
    //}

    //void FragmentAnalyzer::mkPOP(FragmentTransaction* transaction,
    //                             const atomic_proposition& ap) {
    //    for (auto& action : transaction -> getFragmentActions()) {
    //        auto mode = action.getFragmentMode();
    //        switch (mode) {
    //            case ADD :
    //                break;
    //            case REP :
    //                break;
    //            case REM :
    //                break;
    //            defalt :
    //                break;
    //        }
    //    }
    //}

    void FragmentAnalyzer::analyzeBoundedness() {
    }

    void FragmentAnalyzer::translate2FOA() {
        mkFragmentValues();
        mkTransactionValues();
        mkOrderValues();
        mkFragmentVars();
        mkTransactionVars();
        mkOrderVars();
    }

    void FragmentAnalyzer::analyzeReachability() {
    }

    void FragmentAnalyzer::loadASM(AndroidStackMachine* a) {
        this -> a = a;
        this -> a -> minimize();
    }
}

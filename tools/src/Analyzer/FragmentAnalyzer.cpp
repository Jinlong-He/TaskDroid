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
    void permuteAll(vector<T> datas, vector<vector<T> >& pers) {
        do {
            vector<T> n;
            for (size_t j = 0; j < datas.size(); j++) {
                n.emplace_back(datas[j]);
            }
            pers.emplace_back(n);
        }
        while (next_permutation(datas.begin(), datas.end()));
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
                    if (transaction -> isAddTobackStack()) {
                        auto v = new enum_value("bf" + to_string(i) + "_" + to_string(j++));
                        //backTransactionValues.emplace_back(*v);
                        backFragmentValueMap[pair(fragment, transaction)] = v;
                        items.emplace_back(v);
                    }
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

    bool isDel(unordered_set<ID>& check, ID c) {
        if (check.size() == 0) return false;
        if (check.count(c) == 0) return true;
        check.erase(c);
        return isDel(check, c + 1);
    }

    void FragmentAnalyzer::mkOrderValues() {
        vector<pair<enum_value*, ID> > fts;
        for (auto& [f, transactions] : a -> getFragmentTransactionMap()) {
            for (auto transaction : transactions) {
                if (transaction -> isAddTobackStack()) {
                    auto value = transactionValueMap.at(transaction);
                    for (ID i = 0; i < h; i++) fts.emplace_back(pair(value, i));
                }
            }
        }
        for (ID hide = 0; hide <= k; hide++) {
            ID show = k - hide;
            vector<vector<ID> > hideOrders, showOrders;
            vector<vector<pair<ID, pair<enum_value*, ID> > > > newShowOrders;
            getOrders(hide, (ID)-1, hideOrders);
            getOrders(show, (ID)-1, showOrders);
            for (auto& order : showOrders) {
                vector<pair<ID, pair<enum_value*, ID> > > newOrder;
                for (auto& i : order) {
                    newOrder.emplace_back(pair(i, pair(nullptr, -1)));
                }
                newShowOrders.emplace_back(newOrder);
            }
            unordered_set<string> names;
            for (auto& order : hideOrders) {
                vector<vector<pair<ID, pair<enum_value*, ID> > > > newHideOrders;
                getOrders(order, fts, newHideOrders);
                for (auto& hideOrder : newHideOrders) {
                    for (auto& showOrder : newShowOrders) {
                        vector<vector<pair<ID, pair<enum_value*, ID> > > > shuffleOrders;
                        util::shuffle(hideOrder, showOrder, shuffleOrders);
                        for (auto& order : shuffleOrders) {
                            string name = "o";
                            unordered_set<ID> check;
                            for (auto& [i, pair] : order) {
                                if (pair.second != -1) check.insert(pair.second);
                            }
                            if (isDel(check, 0)) continue;
                            for (auto& [i, pair] : order) {
                                if (pair.second == -1) name += "_S";
                                else name += "_H" + to_string(pair.second) + pair.first -> identifier() + "o";
                                if (i != -1) name += to_string(i);
                                else name += "null";
                            }
                            if (names.insert(name).second && name.find("onull") == string::npos) {
                                orders.emplace_back(order);
                                enum_value* v = new enum_value(name);
                                orderValueMap[order] = v;
                                orderValues.emplace_back(*v);
                                items.emplace_back(v);
                            }
                        }
                    }
                }
            }
        }
    }

    void FragmentAnalyzer::mkOrderVars() {
        for (ID i = 0; i < viewNum; i++) {
            auto v = new enum_variable("o" + to_string(i), orderValues.begin(),
                                                           orderValues.end());
            orderVarMap[i] = v;
            add_control_state(foa, *v, *v==nullValue);
            items.emplace_back(v);
        }
    }

    void FragmentAnalyzer::mkTransactionVars() {
        for (ID j = 0; j < k; j++) {
            string name = "bs_" + to_string(j);
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

    void FragmentAnalyzer::getREPOrderAP(ID viewID, ID stackID,
                                         atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            if (order[stackID].second.second == -1 && order[stackID].first != -1) {
                auto& value = *orderValueMap.at(order);
                ap = ap | (var == value);
            }
        }
        if (ap.to_string() == "FALSE") ap = atomic_proposition("TRUE");
    }

    void FragmentAnalyzer::getTopOrderAP(ID viewID, ID stackID,
                                         atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            ID min = order.size();
            if (order[stackID].second.second != -1) continue;
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
        if (ap.to_string() == "FALSE") ap = atomic_proposition("TRUE");
    }

    void FragmentAnalyzer::mkPOPOrder(ID viewID, FragmentTransaction* transaction,
                                      const atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        auto transactionValue = transactionValueMap.at(transaction);
        for (auto& order : orders) {
            auto newOrder = order;
            ID count = 0;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.first == transactionValue && 
                    order[i].second.second == 0) {
                    newOrder[i] = pair(order[i].first, pair(nullptr, -1));
                    count++;
                }
                if (order[i].second.first == transactionValue && 
                    order[i].second.second > 0) {
                    newOrder[i] = pair(order[i].first, 
                                       pair(transactionValue, order[i].second.second - 1));
                }
            }
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.first == nullptr) {
                    auto height = order[i].second.second;
                    if (height == h - 1) {
                        newOrder[i] = pair(-1, pair(nullptr, -1));
                    } else {
                        newOrder[i] = pair(order[i].first, pair(transactionValue, height + 1));
                    }
                }
                if (order[i].second.second != -1) {
                    newOrder[i] = pair(newOrder[i].first + count, newOrder[i].second);
                }
            }
            if (count) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREP_BOrder(ID viewID, FragmentTransaction* transaction,
                                        const atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        auto transactionValue = transactionValueMap.at(transaction);
        for (auto& order : orders) {
            auto newOrder = order;
            ID count = 0;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.second == -1 && order[i].second.first == nullptr &&
                    order[i].first != -1) {
                    newOrder[i] = pair(order[i].first, pair(transactionValue, 0));
                    count++;
                }
            }
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.first == transactionValue) {
                    auto height = order[i].second.second;
                    if (height == h - 1) {
                        newOrder[i] = pair(-1, pair(nullptr, -1));
                    } else {
                        newOrder[i] = pair(order[i].first, pair(transactionValue, height + 1));
                    }
                }
                if (order[i].second.second != -1) {
                    newOrder[i] = pair(newOrder[i].first + count, newOrder[i].second);
                }
            }
            if (count) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREPOrder(ID viewID, const atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            auto newOrder = order;
            bool flag = false;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.second == -1 && order[i].first != -1) {
                    newOrder[i] = pair(-1, pair(nullptr, -1));
                    flag = true;
                }
            }
            if (flag) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREP(FragmentAction* action,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        auto viewID = a -> getViewID(action -> getViewID());
        if (transaction -> isAddTobackStack()) mkREP_BOrder(viewID, transaction, ap);
        else mkREPOrder(viewID, ap);
        auto fragment = action -> getFragment();
        auto& newValue = transaction -> isAddTobackStack() ?
                         *backFragmentValueMap.at(pair(fragment, transaction)) :
                         *fragmentValueMap.at(fragment);
        const auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAP = atomic_proposition("FALSE");
            auto repAP = atomic_proposition("FALSE");
            getTopOrderAP(viewID, i, topAP);
            getREPOrderAP(viewID, i, repAP);
            for (ID j = 1; j < k; j++) {
                auto& var = *map.at(pair(i, j));
                add_transition(foa, var, nullValue, repAP & ap);
            }
            auto& newVar = *map.at(pair(i, 0));
            add_transition(foa, newVar, newValue, topAP & ap);
        }
    }

    void FragmentAnalyzer::mkADD(FragmentAction* action,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        auto viewID = a -> getViewID(action -> getViewID());
        auto fragment = action -> getFragment();
        auto& newValue = transaction -> isAddTobackStack() ?
                         *backFragmentValueMap.at(pair(fragment, transaction)) :
                         *fragmentValueMap.at(fragment);
        const auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAP = atomic_proposition("FALSE");
            getTopOrderAP(viewID, i, topAP);
            for (ID j = 0; j < k; j++) {
                auto& newVar = *map.at(pair(i, j));
                auto p = newVar == nullValue;
                if (j > 0) {
                    auto& var = *map.at(pair(i, j - 1));
                    p = p & var != nullValue;
                }
                add_transition(foa, newVar, newValue, topAP & p & ap);
            }
        }
    }

    void FragmentAnalyzer::mkADDPOP(FragmentAction* action,
                                    FragmentTransaction* transaction,
                                    const atomic_proposition& ap) {
        auto viewID = a -> getViewID(action -> getViewID());
        auto fragment = action -> getFragment();
        auto& value = *backFragmentValueMap.at(pair(fragment, transaction));
        auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAP = atomic_proposition("FALSE");
            getTopOrderAP(viewID, i, topAP);
            for (ID j = 0; j < k; j++) {
                auto& var = *map.at(pair(i, j));
                auto p = var == value;
                add_transition(foa, var, sharpValue, topAP & p & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREPPOP(FragmentAction* action,
                                    FragmentTransaction* transaction,
                                    const atomic_proposition& ap) {
        mkADDPOP(action, transaction, ap);
        mkPOPOrder()
    }

    void FragmentAnalyzer::analyzeBoundedness() {
    }

    void FragmentAnalyzer::translate2FOA() {
        mkFragmentValues();
        mkTransactionValues();
        mkOrderValues();
        mkFragmentVars();
        mkTransactionVars();
        mkOrderVars();
        for (auto& [fragment, transactions] : a -> getFragmentTransactionMap()) {
            for (auto& transaction : transactions) {
                auto& value = *transactionValueMap.at(transaction);
                for (auto action : transaction -> getFragmentActions()) {
                    auto mode = action.getFragmentMode();
                    auto ap = transactionVar == value;
                    switch (mode) {
                        case ADD :
                            mkADD(&action, transaction, ap);
                            break;
                        case REP :
                            mkREP(&action, transaction, ap);
                            break;
                        case REM :
                            break;
                        default :
                            break;
                    }
                }
            }
        }
    }

    void FragmentAnalyzer::analyzeReachability() {
        translate2FOA();
        auto ap = atomic_proposition("TRUE");
        verify_invar_nuxmv(foa, ap, "source");
        //unordered_map<string, vector<string> > trace_table;
        //parse_trace_nuxmv(foa, "result", trace_table);
        //for (auto& value : trace_table.at("t")) {
        //    cout << value << " -> ";
        //}
    }

    void FragmentAnalyzer::loadASM(AndroidStackMachine* a) {
        this -> a = a;
        this -> a -> minimize();
        viewNum = a -> getViewMap().size();
    }
}

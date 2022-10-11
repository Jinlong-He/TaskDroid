#include <iostream>
#include <queue>
#include <cmath>
#include "Analyzer/FragmentAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string, std::queue;
using namespace atl;
using namespace ll;
namespace TaskDroid {

    void FragmentAnalyzer::mkFragmentValues() {
        ID i = 0, j = 0;
        fragmentActionValueMap[nullptr] = &nullValue;
        for (auto& [transaction, map] : initFragmentActionMap) {
            for (auto& [viewID, action] : map) {
                auto v = transaction -> isAddTobackStack() ?
                    new enum_value("bf" + to_string(j++)) :
                    new enum_value("f" + to_string(i++));
                fragmentActionValues.emplace_back(*v);
                items.emplace_back(v);
                fragmentActionValueMap[action] = v;
                const auto& fragments = action -> getFragments();
                fragmentValuesMap[fragments[fragments.size() - 1]].push_back(v);
            }
        }
        for (auto& [f, transactions] : fragmentTransactionMap) {
            for (auto transaction : transactions) {
                for (auto action : transaction -> getHighLevelFragmentActions()) {
                    auto v = transaction -> isAddTobackStack() ?
                        new enum_value("bf" + to_string(j++)) :
                        new enum_value("f" + to_string(i++));
                    fragmentActionValues.emplace_back(*v);
                    items.emplace_back(v);
                    fragmentActionValueMap[action] = v;
                    const auto& fragments = action -> getFragments();
                    fragmentValuesMap[fragments[fragments.size() - 1]].push_back(v);
                }
            }
        }
    }

    void FragmentAnalyzer::mkTransactionValues() {
        ID id = 0;
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto& transaction : transactions) {
                string name = "t" + to_string(id++);
                enum_value* v = new enum_value(name);
                transactionValues.emplace_back(*v);
                transactionValueMap[transaction] = v;
                items.emplace_back(v);
                value2TransactionMap[name] = transaction;
                if (transaction -> isAddTobackStack()) {
                    backTransactionValues.emplace_back(*v);
                    //enum_value* pv = new enum_value("pop_" + name);
                    //transactionValues.emplace_back(*pv);
                    //backTransactionValueMap[transaction] = pv;
                    //items.emplace_back(pv);
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

    bool isDel(unordered_map<enum_value*, unordered_map<ID, unordered_set<ID> > >& checks) {
        bool res = false;
        for (auto& [v, map] : checks) {
            unordered_set<ID> newCheck;
            for (auto& [i, check] : map) {
                if (v == nullptr && i == -1) continue;
                res |= isDel(check, 0);
                newCheck.insert(i);
            }
            res |= isDel(newCheck, 0);
        }
        return res;
    }

    bool isDel(const vector<pair<ID, pair<enum_value*, ID> > >& order) {
        unordered_map<enum_value*, unordered_map<ID, unordered_set<ID> > > checks;
        for (auto& pair : order) {
            checks[pair.second.first][pair.second.second].insert(pair.first);
        }
        return isDel(checks);
    }

    bool isShown(const pair<ID, pair<enum_value*, ID> > & pair) {
        if (pair.second.first == nullptr && pair.second.second == 0) return true;
        return false;
    }

    bool isHiden(const pair<ID, pair<enum_value*, ID> > & pair) {
        if (pair.second.first != nullptr) return true;
        return false;
    }

    bool isNull(const pair<ID, pair<enum_value*, ID> > & pair) {
        if (pair.second.first == nullptr && pair.second.second == -1) return true;
        return false;
    }

    void FragmentAnalyzer::mkOrderValues() {
        vector<pair<ID, pair<enum_value*, ID> > > fts;
        for (ID i = 0; i < h; i++) {
            for (ID j = 0; j < h; j++) {
                for (auto& [f, transactions] : fragmentTransactionMap) {
                    for (auto transaction : transactions) {
                        if (transaction -> isAddTobackStack()) {
                            auto value = transactionValueMap.at(transaction);
                            fts.emplace_back(pair(i, pair(value, j)));
                        }
                    }
                }
            }
            fts.emplace_back(pair(i, pair(nullptr, -1)));
            fts.emplace_back(pair(i, pair(nullptr, 0)));
        }
        vector<vector<pair<ID, pair<enum_value*, ID> > > > coms;
        util::combine(fts, h, coms);
        unordered_set<string> names;
        for (auto& com : coms) {
            if (isDel(com)) continue;
            vector<vector<pair<ID, pair<enum_value*, ID> > > > pers;
            util::permut(com, pers);
            for (auto& order : pers) {
                string name = "o";
                for (auto& [i, pair] : order) {
                    if (pair.first == nullptr) name += "_S";
                    else name += "_H" + to_string(pair.second) + pair.first -> identifier() + "o";
                    if (i != -1 && pair.second != -1) name += to_string(i);
                    if (i != -1 && pair.second == -1) name += "null";
                    if (pair.first == nullptr && pair.second == -1) 
                        i = 0;
                }
                if (names.insert(name).second) {
                    orders.emplace_back(order);
                    enum_value* v = new enum_value(name);
                    orderValueMap[order] = v;
                    orderValues.emplace_back(*v);
                    items.emplace_back(v);
                }
            }
        }
    }

    void FragmentAnalyzer::mkOrderVars() {
        for (ID i = 0; i < viewNum; i++) {
            auto v = new enum_variable("o" + to_string(i), orderValues.begin(),
                                                           orderValues.end());
            orderVarMap[i] = v;
            vector<pair<ID, pair<enum_value*, ID> > > nullOrder(h, pair(0, pair(nullptr, -1)));
            nullOrder[0] = pair(0, pair(nullptr, 0));
            auto& nullValue = *orderValueMap.at(nullOrder);
            add_control_state(foa, *v, *v==nullValue);
            items.emplace_back(v);
        }
    }

    void FragmentAnalyzer::mkTransactionVars() {
        for (ID j = 0; j < h; j++) {
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
        for (ID i = 0; i < viewNum; i++) {
            auto& map = fragmentVarMap[i];
            for (ID l = 0; l < h; l++) {
                for (ID j = 0; j < k; j++) {
                    string name = "f_" + to_string(i) + "_" + to_string(l) + "_" + to_string(j);
                    auto v = new enum_variable(name, fragmentActionValues.begin(),
                                                     fragmentActionValues.end());
                    add_control_state(foa, *v, (*v==nullValue));
                    map[pair(l,j)] = v;
                    items.emplace_back(v);
                }
            }
        }
    }

    void FragmentAnalyzer::getREP_BOrderAP(ID viewID, ID stackID, ID topID,
                                           atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            for (ID i = 0; i < h; i++) {
                if (isNull(order[i]) && i != stackID) break;
                if (isNull(order[i]) && i == stackID &&
                    isShown(order[topID])) {
                    auto& value = *orderValueMap.at(order);
                    ap = ap | (var == value);
                    break;
                }
            }
        }
        if (ap.to_string() == "FALSE") ap = atomic_proposition("TRUE");
    }

    void FragmentAnalyzer::getREPOrderAP(ID viewID, ID stackID,
                                         atomic_proposition& ap) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            if (isShown(order[stackID])) {
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
            if (!isShown(order[stackID])) continue;
            for (ID i = 0; i < order.size(); i++) {
                if (isShown(order[i])) {
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

    void FragmentAnalyzer::getFragmentTopAP(ID viewID, ID stackID,
                                            const vector<enum_value*>& values,
                                            atomic_proposition& ap) {
        auto& map = fragmentVarMap.at(viewID);
        auto pp = atomic_proposition("FALSE");
        for (ID i = 0; i < k; i++) {
            auto& var = *map.at(pair(stackID, i));
            auto p = atomic_proposition("FALSE");
            for (auto value : values) {
                p = var == *value | p;
            }
            if (i < k - 1) {
                auto& newVar = *map.at(pair(stackID, i + 1));
                p = p & newVar == nullValue;
            }
            pp = p | pp;
        }
        if (pp.to_string() == "FALSE") pp = atomic_proposition("TRUE");
        ap = pp & ap;
    }

    void FragmentAnalyzer::getPopTopAP(FragmentTransaction* transaction,
                                       atomic_proposition& ap) {
        auto& value = *transactionValueMap.at(transaction);
        auto pp = atomic_proposition("FALSE");
        for (ID i = 0; i < h; i++) {
            auto& var = *backTransactionVarMap.at(i);
            auto p = var == value;
            if (i < h - 1) {
                auto& newVar = *backTransactionVarMap.at(i + 1);
                p = p & newVar == nullValue;
            }
            pp = p | pp;
        }
        if (pp.to_string() == "FALSE") pp = atomic_proposition("TRUE");
        ap = pp & ap;
    }

    void FragmentAnalyzer::mkPOPOrder(FragmentAction* action,
                                      FragmentTransaction* transaction,
                                      const atomic_proposition& ap) {
        auto viewID = viewMap.at(action -> getViewID());
        auto& var = *orderVarMap.at(viewID);
        auto transactionValue = transactionValueMap.at(transaction);
        for (auto& order : orders) {
            auto newOrder = order;
            ID count = 0;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.first == transactionValue && 
                    order[i].second.second == 0) {
                    newOrder[i] = pair(order[i].first, pair(nullptr, 0));
                    count++;
                }
            }
            for (ID i = 0; i < order.size(); i++) {
                //Show
                if (order[i].second.first == nullptr &&
                    order[i].second.second == 0 &&
                    order[i].first != -1) {
                    auto o = newOrder[i].first;
                    if (o + count < h) newOrder[i] = pair(o + count, order[i].second);
                    else newOrder[i] = pair(0, pair(nullptr, -1));
                }
                //Hide and height > 0
                if (order[i].second.first == transactionValue && 
                    order[i].second.second > 0) {
                    newOrder[i] = pair(order[i].first, 
                                       pair(transactionValue, order[i].second.second - 1));
                }
            }
            if (count) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREP_BOrder(FragmentAction* action,
                                        FragmentTransaction* transaction,
                                        const atomic_proposition& ap) {
        auto viewID = viewMap.at(action -> getViewID());
        auto& var = *orderVarMap.at(viewID);
        auto transactionValue = transactionValueMap.at(transaction);
        for (auto& order : orders) {
            auto newOrder = order;
            ID count = 0;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.second == 0 && order[i].second.first == nullptr &&
                    order[i].first != -1) {
                    newOrder[i] = pair(order[i].first, pair(transactionValue, 0));
                    count++;
                }
            }
            bool flag = true;
            for (ID i = 0; i < order.size(); i++) {
                if (order[i].second.first == transactionValue) {
                    auto height = order[i].second.second;
                    if (height == h - 1) {
                        newOrder[i] = pair(0, pair(nullptr, -1));
                    } else {
                        newOrder[i] = pair(order[i].first, pair(transactionValue, height + 1));
                    }
                }
                if (flag && isNull(order[i])) {
                    newOrder[i] = pair(0, pair(nullptr, 0));
                    flag = false;
                }
            }
            if (count) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREPOrder(FragmentAction* action,
                                      const atomic_proposition& ap) {
        auto viewID = viewMap.at(action -> getViewID());
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            auto newOrder = order;
            bool flag = false;
            for (ID i = 0; i < order.size(); i++) {
                if (isShown(order[i])) {
                    flag = true;
                    if (order[i].first == 0) continue;
                    newOrder[i] = pair(0, pair(nullptr, -1));
                }
            }
            if (flag) {
                auto& value = *orderValueMap.at(order);
                auto& newValue = *orderValueMap.at(newOrder);
                add_transition(foa, var, newValue, var == value & ap);
            }
        }
    }

    void FragmentAnalyzer::mkREP_B(Fragment* sourceFragment,
                                   FragmentAction* action,
                                   FragmentTransaction* transaction,
                                   const atomic_proposition& ap) {
        const auto& sourceValues = fragmentValuesMap.at(sourceFragment);
        auto viewID = viewMap.at(action -> getViewID());
        mkREP_BOrder(action, transaction, ap);
        mkA2B(action, transaction, ap);
        auto& newValue = *fragmentActionValueMap.at(action);
        const auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAPs = atomic_proposition("FALSE");
            for (ID j = 0; j < h; j++) {
                if (j == i) continue;
                auto topAP = atomic_proposition("FALSE");
                getREP_BOrderAP(viewID, i, j, topAP);
                getFragmentTopAP(viewID, j, sourceValues, topAP);
                topAPs = topAPs | topAP;
            }
            auto& newVar = *map.at(pair(i, 0));
            add_transition(foa, newVar, newValue, topAPs & ap);
        }
    }

    void FragmentAnalyzer::mkREP(Fragment* sourceFragment,
                                 FragmentAction* action,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        if (transaction -> isAddTobackStack()) {
            mkREP_B(sourceFragment, action, transaction, ap);
            return;
        }
        const auto& sourceValues = fragmentValuesMap.at(sourceFragment);
        auto viewID = viewMap.at(action -> getViewID());
        mkREPOrder(action, ap);
        auto& newValue = *fragmentActionValueMap.at(action);
        const auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAP = atomic_proposition("FALSE");
            auto repAP = atomic_proposition("FALSE");
            getTopOrderAP(viewID, i, topAP);
            getREPOrderAP(viewID, i, repAP);
            getFragmentTopAP(viewID, i, sourceValues, topAP);
            getFragmentTopAP(viewID, i, sourceValues, repAP);
            for (ID j = 1; j < k; j++) {
                auto& var = *map.at(pair(i, j));
                add_transition(foa, var, nullValue, repAP & ap);
            }
            auto& newVar = *map.at(pair(i, 0));
            add_transition(foa, newVar, newValue, topAP & ap);
        }
    }

    void FragmentAnalyzer::mkA2B(FragmentAction* action,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        auto& newValue = *transactionValueMap.at(transaction);
        for (ID i = 0; i < h; i++) {
            auto& newVar = *backTransactionVarMap.at(i);
            auto p = newVar == nullValue;
            if (i > 0) {
                auto& var = *backTransactionVarMap.at(i - 1);
                p = p & var != nullValue;
            }
            add_transition(foa, newVar, newValue, p & ap);
        }
    }

    void FragmentAnalyzer::mkADD(Fragment* sourceFragment,
                                 FragmentAction* action,
                                 FragmentTransaction* transaction,
                                 const atomic_proposition& ap) {
        const auto& sourceValues = fragmentValuesMap.at(sourceFragment);
        auto viewID = viewMap.at(action -> getViewID());
        auto& newValue = *fragmentActionValueMap.at(action);
        if (transaction -> isAddTobackStack()) mkA2B(action, transaction, ap);
        const auto& map = fragmentVarMap.at(viewID);
        for (ID i = 0; i < h; i++) {
            auto topAP = atomic_proposition("FALSE");
            getTopOrderAP(viewID, i, topAP);
            getFragmentTopAP(viewID, i, sourceValues, topAP);
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
        if (!transaction -> isAddTobackStack()) return;
        auto viewID = viewMap.at(action -> getViewID());
        //auto fragment = action -> getFragment();
        auto& value = *fragmentActionValueMap.at(action);
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
        mkPOPOrder(action, transaction, ap);
    }

    void FragmentAnalyzer::endVars() {
        for (auto& [p, map] : fragmentVarMap) {
            for (auto& [p1, var] : map)
                add_transition(foa, *var, *var, atomic_proposition("TRUE"));
        }
        for (auto& [p, var] : orderVarMap)
            add_transition(foa, *var, *var, atomic_proposition("TRUE"));
        for (auto& [p, var] : backTransactionVarMap)
            add_transition(foa, *var, *var, atomic_proposition("TRUE"));
    }

    void FragmentAnalyzer::translate2FOA() {
        if (isTranslate2Foa) return;
        isTranslate2Foa = true;
        mkFragmentValues();
        mkTransactionValues();
        mkOrderValues();
        mkFragmentVars();
        mkTransactionVars();
        mkOrderVars();
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto& transaction : transactions) {
                auto& value = *transactionValueMap.at(transaction);
                for (auto action : transaction -> getHighLevelFragmentActions()) {
                    auto mode = action -> getFragmentMode();
                    auto ap = transactionVar == value;
                    auto popAP = transactionVar == popValue;
                    getPopTopAP(transaction, popAP);
                    switch (mode) {
                        case ADD :
                            mkADD(fragment, action, transaction, ap);
                            mkADDPOP(action, transaction, popAP);
                            break;
                        case REP :
                            mkREP(fragment, action, transaction, ap);
                            mkREPPOP(action, transaction, popAP);
                            break;
                        case REM :
                            break;
                        default :
                            break;
                    }
                }
            }
        }
        endVars();
    }

    void devide(ID n, ID m, vector<ID>& work, vector<vector<ID> >& res) {
        if (n == 1) {
            auto newWork = work;
            newWork.emplace_back(1);
            res.emplace_back(newWork);
            return;
        }
        if (m == 1) {
            auto newWork = work;
            newWork.emplace_back(n);
            res.emplace_back(newWork);
            return;
        }
        for (ID i = 1; i < n - m + 2; i++) {
            auto newWork = work;
            newWork.emplace_back(i);
            devide(n - i, m - 1, newWork, res);
        }
    }

    void devide(ID n, ID m, vector<vector<ID> >& res) {
        vector<ID> work;
        devide(n, m, work, res);
        for (ID i = 1; i < m; i++) {
            vector<ID> work;
            vector<vector<ID> > newRes;
            devide(n, i, work, newRes);
            unordered_set<vector<ID> > perset;
            for (auto& vec : newRes) {
                vector<ID> newVec = vec;
                for (ID j = 0; j < m - i; j++) newVec.emplace_back(0);
                vector<vector<ID> > pers;
                util::permut(newVec, pers);
                perset.insert(pers.begin(), pers.end());
            }
            res.insert(res.end(), perset.begin(), perset.end());
        }
    }

    void FragmentAnalyzer::getStackAP(ID viewID, ID stackID,
                                     const vector<FragmentAction*>& stack,
                                     atomic_proposition& ap,
                                     int type) {
        auto& map = fragmentVarMap.at(viewID);
        if (stack.size() == 0) {
            ap = atomic_proposition("TRUE");
            for (ID i = 0; i < k; i++) {
                auto& var = *map.at(pair(stackID, i));
                ap = ap & (var == nullValue | var == sharpValue);
            }
            return;
        }
        ID m = stack.size();
        vector<ID> poses;
        for (ID i = 0; i < k; i++) poses.emplace_back(i);
        unordered_set<std::set<ID> > coms;
        util::combine(poses, m, coms);
        for (auto& com : coms) {
            auto pp = atomic_proposition("TRUE");
            unordered_set<ID> visited;
            ID maxPos = 0, minPos = k;
            auto iter = com.begin();
            for (ID j = m - 1; j != -1; j--) {
                ID pos = *(iter++);
                auto p = atomic_proposition("FALSE");
                auto& var = *map.at(pair(stackID, pos));
                auto& value = *fragmentActionValueMap.at(stack[j]);
                pp = (var == value) & pp;
                maxPos = pos > maxPos ? pos : maxPos;
                minPos = pos < maxPos ? pos : minPos;
            }
            for (ID j = 0; j < maxPos; j++) {
                auto& var = *map.at(pair(stackID, j));
                if (com.count(j) == 0)  {
                    if (type == 0 && j < minPos) continue;
                    pp = var == sharpValue & pp;
                }
            }
            if (maxPos < k - 1 && type != 1) {
                auto& var = *map.at(pair(stackID, maxPos + 1));
                pp = (var == nullValue) & pp;
            }
            ap = pp | ap;
        }
    }

    void FragmentAnalyzer::getStackAP(ID viewID, 
                                      const vector<FragmentAction*>& stack,
                                      atomic_proposition& ap,
                                      bool star) {
        auto& var = *orderVarMap.at(viewID);
        for (auto& order : orders) {
            vector<ID> stackOrder(h);
            ID num = 0;
            for (ID i = 0; i < order.size(); i++) {
                if (isShown(order[i])) {
                    stackOrder[order[i].first] = i;
                    num++;
                }
            }
            if (num == 0) continue;
            auto& value = *orderValueMap.at(order);
            auto oAP = var == value;
            stackOrder.resize(num);
            vector<vector<ID> > devs;
            devide(stack.size(), num, devs);
            auto stackAP = atomic_proposition("FALSE");
            for (auto& dev : devs) {
                auto pp = atomic_proposition("TRUE");
                ID index = 0;
                bool flag = true;
                for (ID i = 0; i < dev.size(); i++) {
                    auto p = atomic_proposition("FALSE");
                    vector<FragmentAction*> newStack(stack.begin() + index, 
                                                stack.begin() + index + dev[i]);
                    index += dev[i];
                    if (dev[i] == 0) {
                        if (star && (flag || index == h - 1)) continue;
                        getStackAP(viewID, stackOrder[i], newStack, p);
                    } else {
                        if (flag && star) {
                            getStackAP(viewID, stackOrder[i], newStack, p, 0);
                        } else if (index == h - 1 && star) {
                            getStackAP(viewID, stackOrder[i], newStack, p, 1);
                        } else {
                            getStackAP(viewID, stackOrder[i], newStack, p);
                        }
                        flag = false;
                    }
                    pp = p & pp;
                }
                stackAP = stackAP | pp;
            }
            ap = ap | oAP & stackAP;
        }
    }

    bool FragmentAnalyzer::analyzeReachability(const atomic_proposition& ap,
                                               std::ostream& os) {
        bool flag = false;
        for (auto& [transaction, map] : initFragmentActionMap) {
            clear_init_list(foa);
            for (auto& [viewID, action] : map) {
                auto& var = *fragmentVarMap.at(viewID).at(pair(0,0));
                auto& value = *fragmentActionValueMap.at(action);
                add_init_list(foa, var == value);
            } 
            for (ID i = 0; i < viewNum; i++) {
                for (ID l = 0; l < h; l++) {
                    for (ID j = 0; j < k; j++) {
                        auto& var = *fragmentVarMap.at(i).at(pair(l,j));
                        if (map.count(i) == 0) {
                            add_init_list(foa, var == nullValue);
                        } else {
                            if (l > 0 || j > 0)
                                add_init_list(foa, var == nullValue);
                        }
                    }
                }
                auto& var = *orderVarMap[i];
                vector<pair<ID, pair<enum_value*, ID> > > nullOrder(h, pair(0, pair(nullptr, -1)));
                nullOrder[0] = pair(0, pair(nullptr, 0));
                auto& nullValue = *orderValueMap.at(nullOrder);
                add_init_list(foa, var == nullValue);
            }
            for (ID j = 0; j < h; j++) {
                auto& var = *backTransactionVarMap.at(j);
                add_init_list(foa, var == nullValue);
            }
            std::ifstream f("nuxmv_result");
            if (f) system("rm nuxmv_result");
            verify_invar_nuxmv(foa, ap, "nuxmv_source", 30);
            std::ifstream fin("nuxmv_result");
            if (!fin) {
                flag = flag | false;
                continue;
            }
            unordered_map<string, vector<string> > trace_table;
            parse_trace_nuxmv(foa, "nuxmv_result", trace_table);
            os << "-Fragment Trace Found: " << endl;
            for (auto& value : trace_table.at("t")) {
                if (value == "pop") os << "back()" << endl;
                else os << value2TransactionMap[value] -> toString() << endl;
            }
            flag = flag | true;
        } 
        return flag;
    }

    bool FragmentAnalyzer::analyzeReachability(const string& viewID,
                                               const vector<FragmentAction*>& stack,
                                               std::ostream& os) {
        return analyzeReachability(viewMap.at(viewID), stack, 0, os);
    }

    bool FragmentAnalyzer::analyzeReachability(ID viewID,
                                               const vector<FragmentAction*>& stack, 
                                               bool star, std::ostream& os) {
        auto ap = atomic_proposition("FALSE");
        getStackAP(viewID, stack, ap, star);
        return analyzeReachability(ap, os);
    }

    void FragmentAnalyzer::formalizeGraph(const LGraph& lgraph, 
                                          vector<Graph>& graphs) {
        for (ID i = 0; i < viewMap.size(); i++) {
            Graph graph;
            for (auto& [ver, map] : lgraph) {
                for (auto& [newVer, labels] : map) {
                    int max = -100000;
                    for (auto& label : labels) {
                        if (label.at(i) > max) max = label.at(i);
                    }
                    graph[ver][newVer] = max;
                }
            }
            graphs.emplace_back(graph);
        }
    }

    void FragmentAnalyzer::getGraph(const Vertex& init, LGraph& graph) {
        vector<Vertex> work, newWork;
        work.emplace_back(init);
        while (work.size()) {
            for (auto& ver : work) {
                for (auto& vec : ver) {
                    for (auto top : vec) {
                        if (fragmentTransactionMap.count(top) == 0)  continue;
                        for (auto transaction : fragmentTransactionMap.at(top)) {
                            Vertex newVer = ver;
                            vector<int> label(viewMap.size(), 0);
                            for (auto action : transaction -> getHighLevelFragmentActions()) {
                                auto viewID = viewMap.at(action -> getViewID());
                                auto mode = action -> getFragmentMode();
                                auto& frgs = action -> getFragments();
                                newVer.at(viewID) = frgs;
                                if (mode == ADD) label.at(viewID) = 1;
                                else label.at(viewID) = -100000;
                            }
                            graph[ver][newVer].insert(label);
                            if (graph.count(newVer) == 0) 
                                newWork.emplace_back(newVer);
                        }
                    }
                }
            }
            work.clear();
            if (newWork.size() > 0) work = newWork;
            newWork.clear();
        }
    }

    bool FragmentAnalyzer::analyzeBoundedness(std::ostream& os) {
        bool flag = false;
        for (auto& [transaction, map] : initFragmentActionMap) {
            //auto& actions = transaction -> getFragmentActions();
            flag = false;
            Vertex ver(viewMap.size());
            for (auto [viewID, action] : map) {
                auto mode = action -> getFragmentMode();
                auto& frgs = action -> getFragments();
                ver.at(viewID) = frgs;
                if (mode == ADD) {
                    os << "[Frag-Unboundedness :ADD] Pattern Found:" << endl;
                    os << activity -> getName() << " ADD" << endl;
                    os << "---Pattern END---" << endl;
                    flag = true;
                }
            }
            if (!flag) {
                LGraph graph;
                vector<Graph> graphs;
                getGraph(ver, graph);
                formalizeGraph(graph, graphs);
                vector<vector<Vertex> > loops;
                for (auto& graph : graphs) {
                    LoopAnalyzer<Vertex>::getPositiveLoop(graph, loops);
                    if (loops.size()) flag = true;
                    for (auto& loop : loops) {
                        os << "[Frag-Unboundedness :REP] Pattern Found:" << endl;
                        os << activity -> getName() << " REP" << endl;
                        os << "---Pattern END---" << endl;
                        flag = true;
                    }
                }
            }
        }
        return flag;
    }

    void FragmentAnalyzer::loadASM(AndroidStackMachine* a) {
        this -> a -> formalize();
    }

    void FragmentAnalyzer::loadActivity(Activity* activity) {
        fragmentMap = a -> getFragmentMap(activity);
        fragmentTransactionMap = a -> getFragmentTransactionMap(activity);
        initFragmentActionMap = a -> getInitFragmentActionMap(activity);
        viewMap = a -> getViewMap(activity);
        viewNum = viewMap.size();
        translate2FOA();
    }
}

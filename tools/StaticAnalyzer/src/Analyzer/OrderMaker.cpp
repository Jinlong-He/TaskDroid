#include <iostream>
#include <fstream>
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;
namespace TaskDroid {
    atomic_proposition MultiTaskAnalyzer::getTopOrderAP(ID taskID) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (order[taskID] == 0) ap = ap | orderVar == *v;
        }
        if (ap.to_string() == "FALSE") return atomic_proposition("TRUE");
        return ap;
    }

    atomic_proposition MultiTaskAnalyzer::getTopOrderAP(ID task0ID, ID task1ID, bool eq) {
        atomic_proposition ap("FALSE");
        for (auto& [order, v] : orderValueMap) {
            if (eq && order[task0ID] == 0 && order[task1ID] == 1) 
                ap = ap | orderVar == *v;
            if (!eq && order[task0ID] == 0 && order[task1ID] >= 1) 
                ap = ap | orderVar == *v;
        }
        if (ap.to_string() == "FALSE") return atomic_proposition("TRUE");
        return ap;
    }

    const enum_value& MultiTaskAnalyzer::getSwtOrderValue(const Order& order, 
                                                          ID newTaskID, 
                                                          bool finish) {
        Order newOrder;
        if (finish) {
            Order popOrder;
            getPopOrder(order, popOrder);
            getSwtOrder(popOrder, newTaskID, newOrder);
        } else {
            getSwtOrder(order, newTaskID, newOrder);
        }
        return *orderValueMap.at(newOrder);
    }

    const enum_value& MultiTaskAnalyzer::getPopOrderValue(const Order& order) {
        Order popOrder;
        getPopOrder(order, popOrder);
        return *orderValueMap.at(popOrder);
    }

    void MultiTaskAnalyzer::getSwtOrder(const Order& order, ID newTaskID,
                                        Order& newOrder) {
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

    void MultiTaskAnalyzer::getPopOrder(const Order& order, Order& newOrder) {
        newOrder = order;
        for (ID i = 0; i < order.size(); i++) {
            if (order.at(i) == -1) newOrder[i] = -1;
            else newOrder[i] = order.at(i) - 1;
        }
    }

    void MultiTaskAnalyzer::getTopOrders(ID taskID, vector<Order>& newOrders) {
        for (auto& order : orders)
            if (order.at(taskID) == 0) newOrders.emplace_back(order);
    }

    vector<ID> MultiTaskAnalyzer::getTasks(const Order& order, ID taskID) {
        std::map<ID, ID> taskMap;
        ID min = taskNum, id = 0;
        for (ID i = 0; i < order.size(); i++) {
            if (taskIDMap.count(i) == 0) continue;
            if (order.at(i) != -1 && taskIDMap.at(i) == taskID) 
                taskMap[order.at(i)] = i;
        }
        vector<ID> tasks;
        for (auto& [i, id] : taskMap)
            tasks.emplace_back(id);
        return tasks;
    }
}

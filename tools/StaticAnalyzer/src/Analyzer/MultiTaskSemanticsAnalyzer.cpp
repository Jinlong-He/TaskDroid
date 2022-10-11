#include <iostream>
#include <fstream>
#include <time.h> 
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;

namespace TaskDroid {

    bool MultiTaskAnalyzer::analyzeSemantics(std::ostream& os) {
        auto mainActivity = a -> getMainActivity();
        bool flag = false, sit = false;
        //Lmd(B)=SIT (1)
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                auto target = intent -> getActivity();
                auto lmd = target -> getLaunchMode();
                auto targetID = getTaskID(target);
                cout << lmd << endl;
                if (lmd == SIT) {
                    vector<Order> orders;
                    getTopOrders(mainTaskID, orders);
                    bool sit1 = false, sit2 = false;
                    cout << orders.size() << endl;
                    for (auto& order : orders) {
                        auto tasks = getTasks(order, targetID);
                        if (tasks.size() > 0 && !sit1) {
                            sit1 = true;
                            os << "[Semantics: SIT(1)] Pattern Found:" << endl;
                            os << activity -> getName() << " -> "
                               << target -> getName() << endl;
                            os << "---Pattern END---" << endl;
                            auto orderAP = (orderVar == *orderValueMap.at(order));
                            auto topAP = mkIsTopActAP(mainTaskID, 0, mainActivity);
                            if (analyzeReachability(topAP & orderAP, os)) flag = true;
                        } 
                        if (tasks.size() == 0 && !sit2) {
                            sit2 = true;
                            os << "[Semantics: SIT(2)] Pattern Found:" << endl;
                            os << activity -> getName() << " -> "
                               << target -> getName() << endl;
                            os << "---Pattern END---" << endl;
                            auto orderAP = (orderVar == *orderValueMap.at(order));
                            auto topAP = mkIsTopActAP(mainTaskID, 0, mainActivity);
                            if (analyzeReachability(topAP & orderAP, os)) flag = true;
                        }
                        if (sit1 && sit2) break;
                    }
                    sit = true;
                    break;
                }
            }
            if (sit) break;
        }
        return 1;
    }
};


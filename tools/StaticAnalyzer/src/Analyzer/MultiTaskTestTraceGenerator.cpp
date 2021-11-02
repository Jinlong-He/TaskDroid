#include <iostream>
#include <fstream>
#include <time.h> 
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using std::cout, std::endl, std::to_string;

namespace TaskDroid {

    void MultiTaskAnalyzer::getTransitionNum(Activity* activity, 
                                             Activities& visited,
                                             vector<pair<Activity*, Intent*>>& path) {
        if (a -> getActionMap().count(activity) == 0) return;
        for (auto& [intent, finish] : a -> getActionMap().at(activity)) {
            auto newActivity = intent -> getActivity();
            path.emplace_back(pair(activity, intent));
            if (visited.insert(newActivity).second)
                getTransitionNum(newActivity, visited, path);
        }
    }

    bool less(const pair<Intent*, ID>& p1,
              const pair<Intent*, ID>& p2) {
        return p1.second < p2.second;
    }

    void MultiTaskAnalyzer::devideActions(Activity* activity, ID sum, double p,
                            vector<pair<Activity*, Intent*>>& path,
                            vector<pair<Activity*, Intent*>>& actions,
                            unordered_set<pair<Activity*, Intent*>>& actions1,
                            unordered_set<pair<Activity*, Intent*>>& actions2) {
        auto& map = a -> getActionMap().at(activity);
        if (map.size() == 1) {
            auto intent = map.begin() -> first;
            path.emplace_back(pair(activity, intent));
            devideActions(intent -> getActivity(), sum, p, path, 
                          actions, actions1, actions2);
        } else {
            Activities visited;
            for (auto& trans : path)
                visited.insert(trans.first);
            unordered_map<Intent*, vector<pair<Activity*, Intent*>>> pathMap;
            vector<pair<Intent*, ID>> intents;
            for (auto& [intent, finish] : map) {
                auto newActivity = intent -> getActivity();
                if (a -> getActionMap().count(newActivity) == 0)
                    intents.emplace_back(pair(intent, 1));
                else 
                    intents.emplace_back(pair(intent, a -> getActionMap().at(newActivity).size() + 1));
            }
            sort(intents.begin(), intents.end(), less);
            double pp = p*(sum - path.size());
            cout << pp << endl;
            ID num = 0, id = 0;
            for (ID i = 0; i < intents.size(); i++) {
                num += intents[i].second;
                if (num > pp) {
                    id = i;
                    break;
                }
            }
            cout << id << endl;
            for (ID i = 0; i <= id; i++) {
                auto newActivity = intents[i].first -> getActivity();
                if (a -> getActionMap().count(newActivity) == 0) {
                    actions1.insert(pair(activity, intents[i].first));
                    continue;
                }
                for (auto& t : a -> getActionMap().at(newActivity)) {
                    actions1.insert(pair(newActivity, t.first));
                }
            }
            for (auto& t : actions)
                if (actions1.count(t) == 0) actions2.insert(t);
            for (auto& t : path) actions2.erase(t);
        }
    }

    void MultiTaskAnalyzer::mkTraceActionVars(const unordered_set<pair<Activity*, Intent*>>& actions) {
        ID id = 0;
        if (actions.size() == 0) {
            for (auto& [activity, actions] : a -> getActionMap()) {
                for (auto& [intent, finish]  : actions) {
                    string name = "tb_" + to_string(id++);
                    bool_variable* v = new bool_variable(name);
                    actionBVarMap[pair(activity, intent)] = v;
                    items.emplace_back(v);
                    add_control_state(foa, *v, *v == false);
                }
            }
        } else {
            for (auto& [activity, intent] : actions) {
                string name = "tb_" + to_string(id++);
                bool_variable* v = new bool_variable(name);
                actionBVarMap[pair(activity, intent)] = v;
                items.emplace_back(v);
                add_control_state(foa, *v, *v == false);
            }
        }
    }

    void MultiTaskAnalyzer::mkTraceActionVars(double p) {
        ID id = 0;
        srand((unsigned)time(NULL));
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish]  : actions) {
                double pp = rand() / double(RAND_MAX);
                if (pp > p) continue;
                string name = "tb_" + to_string(id++);
                bool_variable* v = new bool_variable(name);
                actionBVarMap[pair(activity, intent)] = v;
                items.emplace_back(v);
                add_control_state(foa, *v, *v == false);
            }
        }
    }

    void MultiTaskAnalyzer::mkTraceActivityVars(const Activities& acts) {
        ID id = 0;
        for (auto activity : acts) {
            string name = "a_" + to_string(id++);
            bool_variable* v = new bool_variable(name);
            activityBVarMap[activity] = v;
            items.emplace_back(v);
            if (a -> getMainActivity() == activity) {
                add_control_state(foa, *v, *v == true);
            } else {
                add_control_state(foa, *v, *v == false);
            }
        }
        for (auto& [activity, actions] : a -> getActionMap()) {
            for (auto& [intent, finish] : actions) {
                auto& actionValue = *actionValueMap.at(pair(activity, intent));
                auto target = intent -> getActivity();
                if (acts.count(target) > 0) {
                    add_transition(foa, *activityBVarMap[target], bool_value(1), actionVar == actionValue);
                }
            }
        }
        for (auto activity : acts) {
            auto& var = *activityBVarMap[activity];
            add_transition(foa, var, var, atomic_proposition("TRUE"));
        }
    }

    void MultiTaskAnalyzer::genTestTrace(double p, std::ostream& os) {
        if (p == 0) return;
        clear();
        mkVarsValues();
        mkTraceActionVars(unordered_set<pair<Activity*, Intent*>>());
        auto ap = atomic_proposition("TRUE");
        for (auto&[pair, var] : actionBVarMap) ap = ap & (*var == bool_value(1));
        if (!analyzeReachability(ap, os)) {
            auto mainActivity = a -> getMainActivity();
            Activities visited({mainActivity});
            vector<pair<Activity*, Intent*>> path, actions;
            unordered_set<pair<Activity*, Intent*>> actions1, actions2;
            getTransitionNum(mainActivity, visited, actions);
            ID sum = actions.size();
            devideActions(mainActivity, sum, p, path, actions, actions1, actions2);
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            cout << actions1.size() << endl;
            cout << actions2.size() << endl;
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            cout << "-----------" << endl;
            clear();
            mkVarsValues();
            getAvailablePos();
            mkTraceActionVars(actions1);
            auto ap = atomic_proposition("TRUE");
            for (auto&[pair, var] : actionBVarMap) ap = ap & (*var == bool_value(1));
            analyzeReachability(ap, os);
            clear();
            mkVarsValues();
            getAvailablePos();
            mkTraceActionVars(actions2);
            ap = atomic_proposition("TRUE");
            for (auto&[pair, var] : actionBVarMap) ap = ap & (*var == bool_value(1));
            analyzeReachability(ap, os);
        }
    }
}

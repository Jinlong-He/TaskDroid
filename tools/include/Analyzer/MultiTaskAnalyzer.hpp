//
//  MultiTaskAnalyzer.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef MultiTaskAnalyzer_hpp 
#define MultiTaskAnalyzer_hpp 

#include "../AndroidStackMachine/AndroidStackMachine.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
using namespace atl;
using namespace ll;
namespace TaskDroid {
    class MultiTaskAnalyzer {
    public:
        MultiTaskAnalyzer()
            : k(0),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              orderValues({nullValue}),
              actionValues({nullValue, popValue}),
              activityValues({nullValue}) {}

        MultiTaskAnalyzer(ID k_)
            : k(k_),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              orderValues({nullValue}),
              actionValues({nullValue, popValue}),
              activityValues({nullValue}) {}

        ~MultiTaskAnalyzer() {
            for (auto v : items) {
                delete v;
                v = nullptr;
            }
        }

        void loadASM(AndroidStackMachine* a);
        void analyze(AndroidStackMachine* a);
        bool analyzeBoundedness();
        void analyzeReachability(AndroidStackMachine* a, int k, 
                                 const Configuration& configuration);
        void analyzeReachability(const string& affinity, 
                                 const vector<Activity*>& task);
        void translate2FOA();
    private:
        void mkPUSH(Activity* activity, Intent* intent, bool finish, 
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTOP(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTSK(Activity* activity, Intent* intent, bool finish, 
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkSTOP(Activity* activity, Intent* intent, bool finish, 
                    ID taskID, ID actID, const atomic_proposition& ap);

        void mkPUSH_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTOP_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTSK_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const atomic_proposition& ap);
        void mkSTOP_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const atomic_proposition& ap);
        void mkPOP(ID taskID, ID actID, const atomic_proposition& ap);
        void mkOrderValues();
        void mkActivityValues();
        void mkActionValues();
        void mkActivityVars();
        void getTopOrderAP(ID taskID, atomic_proposition& ap);
        void getNewOrder(const vector<ID>& order, ID newTaskID,
                         vector<ID>& newOrder);
        void getPopOrder(const vector<ID>& order, vector<ID>& newOrder);
        enum_value* getMainOrderValue();
        void getTaskAP(const string& affinity, const vector<Activity*>& task,
                       atomic_proposition& ap);
        void setActivity(ID taskID, ID actID, Activity* activity,
                         const atomic_proposition& ap);
        void switchTask(Intent* intent, ID taskID, const atomic_proposition& ap);

        ID k;
        AndroidStackMachine* a;
        fomula_automaton<> foa;

        ID taskNum;
        ID mainTaskID;
        enum_value nullValue;
        enum_value popValue;
        enum_value mainActivityValue;
        enum_variable orderVar;
        enum_variable actionVar;
        vector<item*> items;
        vector<vector<ID> > orders;
        vector<enum_value> orderValues;
        vector<enum_value> actionValues;
        vector<enum_value> activityValues;
        unordered_map<vector<ID>, enum_value*> orderValueMap;
        unordered_map<Activity*, enum_value*> activityValueMap;
        unordered_map<pair<ID, ID>, enum_variable*> activityVarMap;
        unordered_map<pair<Activity*, Intent*>, enum_value*> actionValueMap;
    };
}
#endif /* MultiTaskAnalyzer_hpp */

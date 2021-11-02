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
#include "LoopAnalyzer.hpp"
using namespace atl;
using namespace ll;
namespace TaskDroid {
    class MultiTaskAnalyzer {
    public:
        MultiTaskAnalyzer()
            : k(0),
              isMkVarsValues(false),
              isTranslate2Foa(false),
              isRealAct(false),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              orderValues({nullValue}),
              actionValues({nullValue, popValue}),
              activityValues({nullValue}) {}

        MultiTaskAnalyzer(ID k_, AndroidStackMachine* a_)
            : k(k_),
              isMkVarsValues(false),
              isTranslate2Foa(false),
              isRealAct(false),
              a(a_),
              nullValue("null"),
              popValue("pop"),
              orderValues({nullValue}),
              actionValues({nullValue, popValue}),
              activityValues({nullValue}) {
                  loadASM(a);
              }

        ~MultiTaskAnalyzer() {
            for (auto v : items) {
                delete v;
                v = nullptr;
            }
        }

        void loadASM(AndroidStackMachine* a);
        void analyze(AndroidStackMachine* a);
        bool analyzeBoundedness(std::ostream& os = std::cout);
        bool analyzeBackHijacking(std::ostream& os = std::cout);
        bool analyzeRealActivity(std::ostream& os = std::cout);
        bool analyzeReachability(AndroidStackMachine* a, int k, 
                                 const Configuration<Activity>& configuration,
                                 std::ostream& os = std::cout);
        bool analyzeReachability(const string& affinity, 
                                 const vector<Activity*>& task,
                                 std::ostream& os = std::cout);
        bool analyzeReachability(const atomic_proposition& ap,
                                 std::ostream& os = std::cout);
        bool analyzePattenReachability(const string& affinity, 
                                       const vector<Activity*>& task,
                                       std::ostream& os = std::cout);
        //void genTestTrace(const Activities& acts = Activities(), double p = 1,
        //                  std::ostream& os = std::cout);
        void genTestTrace(double p = 1, std::ostream& os = std::cout);
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
        void setFrontActivity(Activity* activity, const atomic_proposition& ap);
        void setFrontActivity(ID taskID, ID actID, const atomic_proposition& ap);
        void mkVarsValues();
        void mkOrderValues();
        void mkActivityValues();
        void mkActionValues();
        void mkActivityVars();
        void mkRealActivityVars();
        void getTransitionNum(Activity* activity, Activities& visited,
                              vector<pair<Activity*, Intent*>>& path);
        void devideActions(Activity* activity, ID sum, double p,
                           vector<pair<Activity*, Intent*>>& path,
                           vector<pair<Activity*, Intent*>>& actions,
                           unordered_set<pair<Activity*, Intent*>>& actions1,
                           unordered_set<pair<Activity*, Intent*>>& actions2);
        void mkTraceActionVars(const unordered_set<pair<Activity*, Intent*>>& actions);
        void mkTraceActionVars(double p);
        void mkTraceActivityVars(const Activities& acts);
        void getNewOrder(const vector<ID>& order, ID newTaskID,
                         vector<ID>& newOrder);
        void getPopOrder(const vector<ID>& order, vector<ID>& newOrder);
        enum_value* getMainOrderValue();
        atomic_proposition getTopOrderAP(ID taskID);
        atomic_proposition getTopOrderAP(ID task0ID, ID task1ID);
        void getPattenTaskAP(const string& affinity, 
                             const vector<Activity*>& task,
                             atomic_proposition& ap);
        void getBackHijackingAP(Activity* source, Activity* target,
                                atomic_proposition& ap);
        void getTaskAP(const string& affinity, const vector<Activity*>& task,
                       atomic_proposition& ap);
        void getRealActAP(Activity* source, Activity* target,
                          atomic_proposition& ap);
        void setActivity(ID taskID, ID actID, Activity* activity,
                         const atomic_proposition& ap);
        void switchTask(Intent* intent, ID taskID, const atomic_proposition& ap);
        void getAvailablePos();
        void getAvailablePos(Activity* activity, ID taskID, ID actID);
        void getOutActivities(ID affinity, Activity* activity, unordered_map<ID, Activities>& visited);
        void getOutActivities();
        void clear();

        ID k;
        bool isMkVarsValues;
        bool isTranslate2Foa;
        bool isRealAct;
        AndroidStackMachine* a;
        fomula_automaton<> foa;

        ID taskNum;
        ID mainTaskID;
        enum_value nullValue;
        enum_value popValue;
        enum_value mainActivityValue;
        enum_variable orderVar;
        enum_variable actionVar;
        enum_variable frontActivityVar;
        vector<item*> items;
        vector<vector<ID> > orders;
        vector<enum_value> orderValues;
        vector<enum_value> actionValues;
        vector<enum_value> activityValues;
        unordered_map<vector<ID>, enum_value*> orderValueMap;
        unordered_map<Activity*, enum_value*> activityValueMap;
        unordered_map<Activity*, bool_variable*> activityBVarMap;
        unordered_map<ID, enum_variable*> realActivityVarMap;
        unordered_map<pair<Activity*, Intent*>, bool_variable*> actionBVarMap;
        unordered_map<pair<ID, ID>, enum_variable*> activityVarMap;
        unordered_map<pair<Activity*, Intent*>, enum_value*> actionValueMap;
        unordered_map<string, pair<Activity*, Intent*>> value2ActionMap;
        unordered_map<Activity*, unordered_map<ID, unordered_set<ID> > > availablePos;
        unordered_map<ID, Activities> outActivities;
    };
}
#endif /* MultiTaskAnalyzer_hpp */

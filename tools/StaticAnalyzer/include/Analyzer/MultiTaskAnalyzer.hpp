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
//#include "LoopAnalyzer.hpp"
using namespace atl;
using namespace ll;
namespace TaskDroid {
    class MultiTaskAnalyzer {
    public:
    typedef vector<ID> Order;
    typedef unordered_map<Order, unordered_map<ID, vector<pair<bool, atomic_proposition> > > > TaskPropMap;
    typedef unordered_map<Activity*, unordered_map<ID, std::set<ID> > > LegalPosMap;
    typedef unordered_map<Activity*, unordered_map<ID, unordered_map<ID, ID> > > PosRecordMap;
        MultiTaskAnalyzer()
            : k(0),
              m(0),
              isMkVarsValues(false),
              isTranslate2Foa(false),
              isRealAct(false),
              hasRTOF(false),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              orderValues({nullValue}),
              actionValues({nullValue, popValue}),
              activityValues({nullValue}) {}

        MultiTaskAnalyzer(AndroidStackMachine* a_, ID k_ = 5, ID m_ = 2)
            : a(a_),
              k(k_),
              m(m_),
              isMkVarsValues(false),
              isTranslate2Foa(false),
              isRealAct(false),
              hasRTOF(false),
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
        bool analyzeSemantics(std::ostream& os = std::cout);
        bool analyzeUnexpectedness(std::ostream& os = std::cout);
        bool analyzeBoundedness(int k, std::ostream& os = std::cout);
        bool analyzeBackHijacking(std::ostream& os = std::cout);
        bool analyzeRealActivity(std::ostream& os = std::cout);
        bool analyzeUBBP(std::ostream& os = std::cout);
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
        void genTestTrace(const Activities& acts = Activities(), double p = 1,
                          std::ostream& os = std::cout);
        void genTestTrace(double p = 1, std::ostream& os = std::cout);
        void translate2FOA();
    private:
        void setFrontActivity(Activity* activity, const atomic_proposition& ap);
        void setFrontActivity(ID taskID, ID actID, const atomic_proposition& ap);
        void mkVarsValues();
        void mkOrderValues();
        void mkActivityValues();
        void mkActionValues();
        void mkVars();
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
        atomic_proposition getTopOrderAP(ID taskID);
        atomic_proposition getTopOrderAP(ID task0ID, ID task1ID, bool eq = true);
        void getPattenTaskAP(const string& affinity, 
                             const vector<Activity*>& task,
                             atomic_proposition& ap);
        void getBackHijackingAP(Activity* source, Activity* target,
                                atomic_proposition& ap);
        void getTaskAP(const string& affinity, const vector<Activity*>& task,
                       atomic_proposition& ap);
        atomic_proposition getRTF7AP(Activity* source, Activity* target);
        atomic_proposition getRTF11AP(Activity* source, Activity* target);
        atomic_proposition getRealActAP(Activity* source, Activity* target);
        atomic_proposition getSTPAP(Activity* source, Activity* target);
        void getUBBPnewAP(Activity* source, Activity* target,
                          atomic_proposition& ap);
        void getUBBPctpAP(Activity* source, Activity* target,
                          atomic_proposition& ap, bool n = false);
        void setActivity(ID taskID, ID actID, Activity* activity,
                         const atomic_proposition& ap);
        void switchTask(Intent* intent, ID taskID, const atomic_proposition& ap);
        void clear();

        void mkLegalPos();
        void mkLegalPos(Activity* activity, ID taskID, ID actID);
        void getOutActivities(ID affinity, Activity* activity,
                              unordered_map<ID, Activities>& visited);
        void getOutActivities();

        void getTopOrders(ID taskID, vector<Order>& newOrders);

        atomic_proposition mkIsTopActAP(ID taskID, ID actID, Activity* act);
        atomic_proposition mkIsTopNullAP(ID taskID, ID actID);
        atomic_proposition mkIsTopNonNullAP(ID taskID, ID actID);
        atomic_proposition mkIsTopTaskAP(ID taskID);
        atomic_proposition mkIsTopTaskAP(ID taskID1, ID taskID2);
        const enum_value& getSwtOrderValue(const Order& order, ID taskID, 
                                           bool finish = false);
        const enum_value& getPopOrderValue(const Order& order);
        void getSwtOrder(const Order& order, ID newTaskID, Order& newOrder);
        void getPopOrder(const Order& order, Order& newOrder);

        vector<ID> getTasks(const Order& order, ID taskID);
        enum_value* getMainOrderValue();

        void mkADDA(ID taskID, ID actID, Activity* activity,
                    const atomic_proposition& ap);
        void mkDELA(ID taskID, ID actID, const atomic_proposition& ap);
        void mkSWTK(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap,
                    TaskPropMap& taskPropMap);
        void mkPOPT(ID taskID, const atomic_proposition& ap);
        void mkPOP();

        void mkPUSH(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkSTOP(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkRTOF(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTOP(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkCTSK(Activity* activity, Intent* intent, bool finish, 
                    ID taskID, ID actID, const atomic_proposition& ap);

        void mkPUSH_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const TaskPropMap& taskPropMap);
        void mkSTOP_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const TaskPropMap& taskPropMap);
        void mkCTOP_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const TaskPropMap& taskPropMap);
        void mkCTSK_N(Activity* activity, Intent* intent, bool finish,
                      ID taskID, ID actID, const TaskPropMap& taskPropMap);

        void mkMKTK(Activity* activity, Intent* intent, bool finish,
                    ID taskID, ID actID, const atomic_proposition& ap);
        void mkSIST(Activity* activity, Intent* intent, bool finish, 
                    ID taskID, ID actID, const atomic_proposition& ap);
                    


        ID getTaskID(Activity* activity);

        ID k;
        ID m;

        ID taskNum;
        ID mainTaskID;

        unordered_set<ID> sitIDSet;
        unordered_map<ID, Activities> outActivities;
        unordered_map<ID, ID> taskIDMap;
        unordered_map<ID, ID> multiTaskIDMap;

        enum_value nullValue;
        enum_value popValue;
        enum_value mainActivityValue;
        enum_variable orderVar;
        enum_variable actionVar;

        bool_variable mainTaskVar;

        vector<item*> items;
        vector<Order> orders;
        unordered_map<pair<ID, ID>, enum_variable*> activityVarMap;
        unordered_map<ID, enum_variable*> realActivityVarMap;

        vector<enum_value> orderValues;
        vector<enum_value> actionValues;
        vector<enum_value> activityValues;
        unordered_map<vector<ID>, enum_value*> orderValueMap;
        unordered_map<Activity*, enum_value*> activityValueMap;
        unordered_map<pair<Activity*, Intent*>, enum_value*> actionValueMap;
        unordered_map<string, pair<Activity*, pair<Intent*, bool>>> value2ActionMap;
        LegalPosMap legalPos;
        PosRecordMap records;
        unordered_map<ID, Activities> visited;
        ID legalPosSize;

        bool isMkVarsValues;
        bool isTranslate2Foa;
        bool isRealAct;
        bool hasRTOF;
        AndroidStackMachine* a;
        fomula_automaton<> foa;

    };
}
#endif /* MultiTaskAnalyzer_hpp */



//
//  FragmentAnalyzer.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef FragmentAnalyzer_hpp 
#define FragmentAnalyzer_hpp 

#include "../AndroidStackMachine/AndroidStackMachine.hpp"
#include "atl/fomula_automaton/fomula_automaton.hpp"
#include "LoopAnalyzer.hpp"
using namespace atl;
using namespace ll;
namespace TaskDroid {
    class FragmentAnalyzer {
    public:
        FragmentAnalyzer()
            : k(0),
              h(0),
              isTranslate2Foa(false),
              a(nullptr),
              activity(nullptr),
              nullValue("null"),
              popValue("pop"),
              sharpValue("sharp"),
              transactionValues({nullValue, popValue}),
              backTransactionValues({nullValue, sharpValue}),
              fragmentActionValues({nullValue, sharpValue}) {}

        FragmentAnalyzer(ID k_, ID h_, 
                         AndroidStackMachine* a_, Activity* activity_)
            : k(k_),
              h(h_),
              isTranslate2Foa(false),
              a(a_),
              activity(activity_),
              nullValue("null"),
              popValue("pop"),
              sharpValue("sharp"),
              transactionValues({nullValue, popValue}),
              backTransactionValues({nullValue, sharpValue}),
              fragmentActionValues({nullValue, sharpValue}) {
                  loadASM(a);
                  loadActivity(activity);
              }
        ~FragmentAnalyzer() {
            for (auto v : items) {
                delete v;
                v = nullptr;
            }
        }

        bool analyzeBoundedness(std::ostream& os = std::cout);
        bool analyzeUBBP(std::ostream& os = std::cout);
        //void checkADD(std::ostream& os = std::cout);
        //void checkREP(std::ostream& os = std::cout);
        void analyzeReachability(std::ostream& os = std::cout);
        bool analyzeReachability(const atomic_proposition& ap,
                                 std::ostream& os = std::cout);
        bool analyzeReachability(const string& viewID,
                                 const vector<FragmentAction*>& task,
                                 std::ostream& os = std::cout);
        bool analyzeReachability(const char* config, std::ostream& os = std::cout);
        bool analyzeReachability(ID viewID, const vector<FragmentAction*>& task, 
                                 bool star = false,
                                 std::ostream& os = std::cout);
        void loadASM(AndroidStackMachine* a);
        void setInitFragments(const unordered_map<string, Fragment*>& initFragments);
        typedef vector<vector<Fragment*> > Vertex;
        typedef vector<int> Label;
        typedef unordered_set<Label> Labels;
        typedef unordered_map<Vertex, unordered_map<Vertex, Labels>> LGraph;
        typedef unordered_map<Vertex, unordered_map<Vertex, int>> Graph;
    private:
        void loadActivity(Activity* activity);
        void mkFragmentValues();
        void mkOrderValues();
        void mkOrderValues1();
        void mkTransactionValues();
        void mkFragmentVars();
        void mkOrderVars();
        void mkTransactionVars();
        void mkADD(Fragment* sourceFragment, FragmentAction* action, 
                   FragmentTransaction* transaction,
                   const atomic_proposition& ap);
        void mkREP(Fragment* sourceFragment, FragmentAction* action, 
                   FragmentTransaction* transaction,
                   const atomic_proposition& ap);
        void mkREP_B(Fragment* sourceFragment, FragmentAction* action, 
                   FragmentTransaction* transaction,
                   const atomic_proposition& ap);
        void mkADDPOP(FragmentAction* action, FragmentTransaction* transaction,
                     const atomic_proposition& ap);
        void mkREPPOP(FragmentAction* action, FragmentTransaction* transaction,
                     const atomic_proposition& ap);

        void mkA2B(FragmentAction* action, FragmentTransaction* transaction,
                   const atomic_proposition& ap);

        void endVars();

        void translate2FOA();
        void getTopOrderAP(ID viewID, ID stackID, atomic_proposition& ap);
        void getREPOrderAP(ID viewID, ID stackID, atomic_proposition& ap);
        void getREP_BOrderAP(ID viewID, ID stackID, ID topID,
                             atomic_proposition& ap);
        void getPopTopAP(FragmentTransaction* transaction, 
                         atomic_proposition& ap);
        void getFragmentTopAP(ID viewID, ID taskID, 
                              const vector<enum_value*>& values,
                              atomic_proposition& ap);
        void mkREPOrder(FragmentAction* action, const atomic_proposition& ap);
        void mkREP_BOrder(FragmentAction* action, 
                          FragmentTransaction* transaction, 
                          const atomic_proposition& ap);
        void mkPOPOrder(FragmentAction* action, 
                        FragmentTransaction* transaction, 
                        const atomic_proposition& ap);

        void getStackAP(ID viewID, ID stackID, 
                        const vector<FragmentAction*>& task,
                        atomic_proposition& ap, int type = -1);
        void getStackAP(ID viewID, const vector<FragmentAction*>& task,
                        atomic_proposition& ap, bool star = false);
        void getGraph(const Vertex& init, LGraph& graph);
        void formalizeGraph(const LGraph& lgraph, vector<Graph>& graphs);
        void getUBBPAP(FragmentTransaction* transaction, 
                       atomic_proposition& ap);

        ID k;
        ID h;
        bool isTranslate2Foa;
        AndroidStackMachine* a;
        Activity* activity;
        fomula_automaton<> foa;

        FragmentMap fragmentMap;
        FragmentTransactionMap fragmentTransactionMap;
        InitFragmentActionMap initFragmentActionMap;
        ViewMap viewMap;

        ID viewNum;

        FragmentTransactionMap backTransactionMap;
        enum_value nullValue;
        enum_value popValue;
        enum_value sharpValue;
        enum_variable transactionVar;
        vector<item*> items;
        vector<vector<pair<ID, pair<enum_value*, ID> > > > orders;
        vector<enum_value> transactionValues;
        vector<enum_value> backTransactionValues;
        vector<enum_value> fragmentValues;
        vector<enum_value> fragmentActionValues;
        vector<enum_value> orderValues;
        unordered_map<string, ID> viewIDMap;
        unordered_map<vector<pair<ID, pair<enum_value*, ID> > >, enum_value*> orderValueMap;
        unordered_map<Fragment*, enum_value*> fragmentValueMap;
        unordered_map<Fragment*, vector<enum_value*> > fragmentValuesMap;
        unordered_map<FragmentTransaction*, enum_value*> transactionValueMap;
        unordered_map<pair<Fragment*, FragmentTransaction*>, enum_value*> backFragmentValueMap;
        unordered_map<ID, 
            unordered_map<pair<ID, ID>, enum_variable*> > fragmentVarMap;
        unordered_map<ID, enum_variable*> backTransactionVarMap;
        unordered_map<ID, enum_variable*> orderVarMap;
        unordered_map<string, FragmentTransaction*> value2TransactionMap;

        unordered_map<FragmentAction*, vector<enum_value*> > fragmentActionValuesMap;
        unordered_map<FragmentAction*, enum_value*> fragmentActionValueMap;
        unordered_map<pair<FragmentAction*, FragmentTransaction*>, enum_value*> backFragmentActionValueMap;
    };
}
#endif /* FragmentAnalyzer_hpp */

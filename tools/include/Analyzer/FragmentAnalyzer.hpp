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
using namespace atl;
using namespace ll;
namespace TaskDroid {
    class FragmentAnalyzer {
    public:
        FragmentAnalyzer()
            : k(0),
              h(0),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              sharpValue("sharp"),
              transactionValues({nullValue, popValue}),
              backTransactionValues({nullValue}),
              fragmentValues({nullValue, sharpValue}) {}

        FragmentAnalyzer(ID k_, ID h_)
            : k(k_),
              h(h_),
              a(nullptr),
              nullValue("null"),
              popValue("pop"),
              sharpValue("sharp"),
              transactionValues({nullValue, popValue}),
              backTransactionValues({nullValue}),
              fragmentValues({nullValue, sharpValue}) {}

        void analyzeBoundedness();
        void analyzeReachability(Activity* activity, const string& viewID,
                                 const vector<Fragment*>& task);
        void analyzeReachability(Activity* activity);
        void loadASM(AndroidStackMachine* a);
        void setInitFragments(const unordered_map<string, Fragment*>& initFragments);
    private:
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
        void getFragmentTopAP(ID viewID, ID taskID, Fragment* fragment,
                              atomic_proposition& ap);
        void mkREPOrder(FragmentAction* action, const atomic_proposition& ap);
        void mkREP_BOrder(FragmentAction* action, 
                          FragmentTransaction* transaction, 
                          const atomic_proposition& ap);
        void mkPOPOrder(FragmentAction* action, 
                        FragmentTransaction* transaction, 
                        const atomic_proposition& ap);

        void getStackAP(ID viewID, ID stackID, const vector<Fragment*>& task,
                       atomic_proposition& ap);
        void getStackAP(const string& viewID, const vector<Fragment*>& task,
                       atomic_proposition& ap);

        ID k;
        ID h;
        AndroidStackMachine* a;
        fomula_automaton<> foa;

        FragmentMap fragmentMap;
        FragmentTransactionMap fragmentTransactionMap;
        InitFragmentsMap initFragmentsMap;
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


    };
}
#endif /* FragmentAnalyzer_hpp */

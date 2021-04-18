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
              sharpValue("sharp"),
              popValue("pop"),
              orderValues({nullValue}),
              transactionValues({nullValue, popValue}),
              fragmentValues({nullValue}) {}

        FragmentAnalyzer(ID k_, ID h_)
            : k(k_),
              h(h_),
              a(nullptr),
              nullValue("null"),
              sharpValue("sharp"),
              popValue("pop"),
              orderValues({nullValue}),
              transactionValues({nullValue, popValue}),
              fragmentValues({nullValue}) {}

        void analyzeBoundedness();
        void analyzeReachability();
        void loadASM(AndroidStackMachine* a);
    private:
        void mkFragmentValues();
        void mkOrderValues();
        void mkTransactionValues();
        void mkFragmentVars();
        void mkOrderVars();
        void mkTransactionVars();
        void mkADD(FragmentAction* action, FragmentTransaction* transaction,
                   const atomic_proposition& ap);
        void mkREP(FragmentAction* action, FragmentTransaction* transaction,
                   const atomic_proposition& ap);
        void mkREP_B(FragmentAction* action, FragmentTransaction* transaction,
                     const atomic_proposition& ap);
        void mkADDPOP(FragmentAction* action, FragmentTransaction* transaction,
                     const atomic_proposition& ap);
        void mkREPPOP(FragmentAction* action, FragmentTransaction* transaction,
                     const atomic_proposition& ap);

        void translate2FOA();
        void getTopOrderAP(ID viewID, ID stackID, atomic_proposition& ap);
        void getREPOrderAP(ID viewID, ID stackID, atomic_proposition& ap);
        void mkREPOrder(ID viewID, const atomic_proposition& ap);
        void mkREP_BOrder(ID viewID, FragmentTransaction* transaction, const atomic_proposition& ap);
        void mkPOPOrder(ID viewID, FragmentTransaction* transaction, const atomic_proposition& ap);

        ID k;
        ID h;
        AndroidStackMachine* a;
        fomula_automaton<> foa;

        ID viewNum;

        FragmentTransactionMap backTransactionMap;
        enum_value nullValue;
        enum_value sharpValue;
        enum_value popValue;
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
        unordered_map<ID, 
            unordered_map<pair<ID, ID>, enum_variable*> > fragmentVarMap;
        unordered_map<ID, enum_variable*> backTransactionVarMap;
        unordered_map<ID, enum_variable*> orderVarMap;
        unordered_map<FragmentTransaction*, enum_value*> transactionValueMap;
        unordered_map<pair<Fragment*, FragmentTransaction*>, enum_value*> backFragmentValueMap;
    };
}
#endif /* FragmentAnalyzer_hpp */

//
//  FragmentTransaction.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef FragmentTransaction_hpp 
#define FragmentTransaction_hpp 

#include <vector>
#include "Fragment.hpp"
using std::vector;
namespace TaskDroid {
    enum Action {ADD, REM, REP, HID, SHO};
    typedef size_t ID;
    class FragmentTransaction {
    public:
        struct FragmentAction {
            FragmentAction()
                : action(ADD),
                  fragment(nullptr),
                  viewID(-1) {}
            FragmentAction(Action action_, Fragment* fragment_, ID viewID_ = -1)
                : action (action_),
                  fragment(fragment_),
                  viewID(viewID_) {}
        private:
            Action action;
            Fragment* fragment;
            ID viewID;
        };
        typedef vector<FragmentAction> FragmentActions;

        FragmentTransaction()
            : addToBackStack(0),
              fragmentActions() {}
        FragmentTransaction(bool addToBackStack_, const FragmentActions& fragmentActions_)
            : addToBackStack(addToBackStack_),
              fragmentActions(fragmentActions_) {}
        ~FragmentTransaction() {}

        bool isAddTobackStack() const;
        void setAddTobackStack(bool addToBackStack);
        const FragmentActions& getFragmentActions() const;
        void setFragmentActions(const FragmentActions& fragmentActions);
        void addFragmentAction(Action action, Fragment* fragment, ID viewID = -1);
    private:
        bool addToBackStack;
        FragmentActions fragmentActions;
    };
    typedef FragmentTransaction::FragmentActions FragmentActions;
}
#endif /* FragmentTransaction_hpp */

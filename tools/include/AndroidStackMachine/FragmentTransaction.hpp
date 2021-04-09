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
    enum FragmentMode {ADD, REM, REP};
    typedef size_t ID;
    class FragmentTransaction {
    public:
        struct FragmentAction {
            FragmentAction()
                : mode(ADD),
                  fragment(nullptr),
                  viewID("") {}
            FragmentAction(FragmentMode mode_, Fragment* fragment_, 
                           const string viewID_ = "")
                : mode (mode_),
                  fragment(fragment_),
                  viewID(viewID_) {}
            FragmentMode getFragmentMode() const {
                return mode;
            }

            Fragment* getFragment() const {
                return fragment;
            }

            const string getViewID() const {
                return viewID;
            }
        private:
            FragmentMode mode;
            Fragment* fragment;
            string viewID;
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
        void addFragmentAction(FragmentMode mode, Fragment* fragment, const string viewID = "");
    private:
        bool addToBackStack;
        FragmentActions fragmentActions;
    };
    typedef FragmentTransaction::FragmentAction FragmentAction;
    typedef FragmentTransaction::FragmentActions FragmentActions;
}
#endif /* FragmentTransaction_hpp */

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
                           const string viewID_ = "",
                           const vector<Fragment*> fragments_ = vector<Fragment*>())
                : mode (mode_),
                  fragment(fragment_),
                  fragments(fragments_),
                  viewID(viewID_) {}

            FragmentMode getFragmentMode() const {
                return mode;
            }

            Fragment* getFragment() const {
                return fragment;
            }

            const vector<Fragment*>& getFragments() const {
                return fragments;
            }

            const string& getViewID() const {
                return viewID;
            }

            string toString() const {
                string res = "";
                if (mode == ADD) res += "ADD(";
                if (mode == REP) res += "REP(";
                res += viewID + "," + fragment -> getName() + ")";
                return res;
            }
        private:
            FragmentMode mode;
            Fragment* fragment;
            vector<Fragment*> fragments;
            string viewID;
        };
        typedef vector<FragmentAction*> FragmentActions;

        FragmentTransaction()
            : addToBackStack(0),
              fragmentActions() {}
        FragmentTransaction(bool addToBackStack_, const FragmentActions& fragmentActions_)
            : addToBackStack(addToBackStack_),
              fragmentActions(fragmentActions_) {}
        ~FragmentTransaction() {
            for (auto action : fragmentActions) {
                delete action;
                action = nullptr;
            }
            for (auto action : highlevelFragmentActions) {
                delete action;
                action = nullptr;
            }
        }

        bool isAddTobackStack() const;
        void setAddTobackStack(bool addToBackStack);
        const FragmentActions& getFragmentActions() const;
        const FragmentActions& getHighLevelFragmentActions() const;
        void setFragmentActions(const FragmentActions& fragmentActions);
        void addFragmentAction(FragmentMode mode, Fragment* fragment, const string viewID = "");
        void addHighLevelFragmentAction(FragmentMode mode, Fragment* fragment, const string viewID = "", const vector<Fragment*>& fragments = vector<Fragment*>());
        void clear();
        string toString() const;
    private:
        bool addToBackStack;
        FragmentActions fragmentActions;
        FragmentActions highlevelFragmentActions;
    };
    typedef FragmentTransaction::FragmentAction FragmentAction;
    typedef FragmentTransaction::FragmentActions FragmentActions;
}
#endif /* FragmentTransaction_hpp */

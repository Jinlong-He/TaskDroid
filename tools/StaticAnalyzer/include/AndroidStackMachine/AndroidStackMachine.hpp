//
//  AndroidStackMachine.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef AndroidStackMachine_hpp 
#define AndroidStackMachine_hpp 

#include <iostream>
#include "Configuration.hpp"
#include "Intent.hpp"
using std::unordered_map, std::unordered_set, std::pair, std::cout, std::endl;
namespace TaskDroid {
    typedef unordered_map<string, Fragment*> FragmentMap;
    typedef vector<FragmentTransaction*> FragmentTransactionVec;
    typedef unordered_map<Fragment*, FragmentTransactionVec> FragmentTransactionMap;
    typedef unordered_map<Activity*, FragmentTransactionVec> ActivityTransactionMap;
    typedef unordered_set<Activity*> Activities;
    typedef unordered_set<Intent*> Intents;
    typedef unordered_set<Fragment*> Fragments;
    typedef unordered_set<FragmentTransaction*> FragmentTransactions;
    typedef vector<Fragment*> FragmentVec;
    typedef vector<Activity*> ActivityVec;
    typedef vector<Intent*> IntentVec;
    typedef pair<Intent*, bool> Action;
    typedef vector<Action> Actions;
    typedef unordered_map<Activity*, Actions> ActionMap;
    typedef unordered_map<string, Activity*> ActivityMap;
    typedef unordered_map<string, ID> AffinityMap;
    typedef unordered_map<string, ID> ViewMap;
    typedef unordered_map<FragmentTransaction*,
                unordered_map<ID, FragmentAction*> > InitFragmentActionMap;
    enum ActionMode {PUSH, STOP, CTOP, CTSK, RTOF, SIST,
                     PUSH_N, STOP_N, CTOP_N, CTSK_N, RTOF_N};
    class AndroidStackMachine {
    public:
        AndroidStackMachine()
            : mainActivity(nullptr),
              activityMap(),
              actionMap(),
              intents(),
              isFomalize(false) {}
        AndroidStackMachine(Activity* mainActivity_, 
                            const ActivityMap& activityMap_,
                            const ActionMap& actionMap_,
                            const IntentVec& intents_,
                            const FragmentVec& fragments_)
            : mainActivity(mainActivity_),
              activityMap(activityMap_),
              actionMap(actionMap_),
              intents(),
              isFomalize(false) {}

        ~AndroidStackMachine() {
            for (auto& pair : activityMap) {
                delete pair.second;
                pair.second = nullptr;
            }
            for (auto intent : intents) {
                delete intent;
                intent = nullptr;
            }
            for (auto& pair : fragmentMap) {
                delete pair.second;
                pair.second = nullptr;
            }
            for (auto ft : fragmentTransactions) {
                delete ft;
                ft = nullptr;
            }
        }
        Activity* mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode);
        Intent* mkIntent(Activity* activity);
        Fragment* mkFragment(const string& name);
        Activity* getActivity(const string& name) const;
        Fragment* getFragment(const string& name) const;
        void setPackageName(const string& packegeName);
        const string& getPackageName() const;
        void setMainActivity(Activity* activity);
        Activity* getMainActivity() const;
        const AffinityMap& getAffinityMap() const;
        const ActivityMap& getActivityMap() const;
        const ActionMap& getActionMap() const;
        const ViewMap& getViewMap() const;
        ID getTaskID(const string& affinity) const;
        ID getViewID(const string& view) const;
        void addAction(Activity* activity, Intent* intent, bool finish = false);
        bool minimize();
        void fomalize();
        void print(std::ostream& os = std::cout) const;
        static ActionMode getMode(Intent* intent);
        static bool isNewMode(Intent* intent);

        const FragmentMap& getFragmentMap() const;
        const FragmentMap& getFragmentMap(Activity* activity) const;
        const FragmentTransactionMap& getFragmentTransactionMap() const;
        const FragmentTransactionMap& getFragmentTransactionMap(Activity* activity) const;
        const ActivityTransactionMap& getActivityTransactionMap() const;
        const InitFragmentActionMap& getInitFragmentActionMap(Activity* activity) const;
        const ViewMap& getViewMap(Activity* activity) const;

        void setFragmentTransactionMap(const FragmentTransactionMap& fragmentTransactionMap);
        void addFragmentTransaction(Fragment* fragment, FragmentTransaction* fragmentTransaction);
        void addFragmentTransaction(Activity* activity, FragmentTransaction* fragmentTransaction);
        FragmentTransaction* mkFragmentTransaction();
        void formActivity();
        void formActivity(Fragment* fragment, FragmentTransactionMap& map,
                          FragmentMap& fragmentMap);
        void formFragmentTransaction(FragmentTransaction* transaction);
    private:
        string packageName;
        Activity* mainActivity;
        ActivityMap activityMap;
        ActionMap actionMap;
        AffinityMap affinityMap;
        ViewMap viewMap;
        FragmentMap fragmentMap;
        Intents intents;
        ActivityTransactionMap activityTransactionMap;
        FragmentTransactionMap fragmentTransactionMap;
        FragmentTransactions fragmentTransactions;
        unordered_map<Activity*, FragmentMap> activity2FragmentsMap;
        unordered_map<Activity*, InitFragmentActionMap> activity2InitFragmentActionMap;
        unordered_map<Activity*, FragmentTransactionMap> 
            activity2FragmentTransactionMap;
        unordered_map<Activity*, ViewMap> 
            activity2ViewMap;
        bool isFomalize;
    };
}
#endif /* AndroidStackMachine_hpp */

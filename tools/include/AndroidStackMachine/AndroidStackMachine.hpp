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

#include "Configuration.hpp"
#include "Intent.hpp"
using std::unordered_map, std::unordered_set, std::pair;
namespace TaskDroid {
    typedef unordered_map<string, Fragment*> FragmentMap;
    typedef vector<FragmentTransaction*> FragmentTransactionVec;
    typedef unordered_map<Fragment*, FragmentTransactionVec> FragmentTransactionMap;
    typedef unordered_map<Activity*, FragmentTransactionVec> ActivityTransactionMap;
    typedef unordered_set<Activity*> Activities;
    typedef unordered_set<Intent*> Intents;
    typedef unordered_set<Fragment*> Fragments;
    typedef vector<Fragment*> FragmentVec;
    typedef vector<Activity*> ActivityVec;
    typedef vector<Intent*> IntentVec;
    typedef pair<Intent*, bool> Action;
    typedef vector<Action> Actions;
    typedef unordered_map<Activity*, Actions> ActionMap;
    typedef unordered_map<string, Activity*> ActivityMap;
    typedef unordered_map<string, ID> AffinityMap;
    typedef unordered_map<string, ID> ViewMap;
    typedef unordered_map<ID, FragmentVec> InitFragmentsMap;
    enum ActionMode {PUSH, STOP, CTOP, CTSK, RTOF,
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
        }
        Activity* mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode);
        Intent* mkIntent(Activity* activity);
        Fragment* mkFragment(const string& name);
        Activity* getActivity(const string& name) const;
        Fragment* getFragment(const string& name) const;
        void setMainActivity(Activity* activity);
        Activity* getMainActivity() const;
        const AffinityMap& getAffnityMap() const;
        const ActivityMap& getActivityMap() const;
        const ActionMap& getActionMap() const;
        const ViewMap& getViewMap() const;
        ID getTaskID(const string& affinity) const;
        ID getViewID(const string& view) const;
        void addAction(Activity* activity, Intent* intent, bool finish = false);
        void minimize();
        void fomalize();
        void print() const;
        static ActionMode getMode(Intent* intent);
        static bool isNewMode(Intent* intent);

        const FragmentMap& getFragmentMap() const;
        const FragmentMap& getFragmentMap(Activity* activity) const;
        const FragmentTransactionMap& getFragmentTransactionMap() const;
        const FragmentTransactionMap& getFragmentTransactionMap(Activity* activity) const;
        const ActivityTransactionMap& getActivityTransactionMap() const;
        const InitFragmentsMap& getInitFragmentsMap(Activity* activity) const;
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
        Activity* mainActivity;
        ActivityMap activityMap;
        ActionMap actionMap;
        AffinityMap affinityMap;
        ViewMap viewMap;
        FragmentMap fragmentMap;
        IntentVec intents;
        ActivityTransactionMap activityTransactionMap;
        FragmentTransactionMap fragmentTransactionMap;
        FragmentTransactionVec fragmentTransactions;
        unordered_map<Activity*, FragmentMap> activity2FragmentsMap;
        unordered_map<Activity*, InitFragmentsMap> activity2InitFragmentsMap;
        unordered_map<Activity*, FragmentTransactionMap> 
            activity2FragmentTransactionMap;
        unordered_map<Activity*, ViewMap> 
            activity2ViewMap;
        bool isFomalize;
    };
}
#endif /* AndroidStackMachine_hpp */

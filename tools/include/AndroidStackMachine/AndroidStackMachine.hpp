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

#include "Activity.hpp"
#include "Intent.hpp"
using boost::unordered_map, boost::unordered_set;
namespace TaskDroid {
    typedef unordered_set<Activity*> Activities;
    typedef unordered_set<Intent*> Intents;
    typedef vector<Fragment*> FragmentVec;
    typedef vector<Activity*> ActivityVec;
    typedef vector<Intent*> IntentVec;
    typedef unordered_map<Activity*, Intents> ActionMap;
    typedef unordered_map<string, Activity*> ActivityMap;
    enum ActionMode {PUSH, STOP, CTOP, CTSK, RTOF,
                     PUSH_A, STOP_A, CTOP_A, CTSK_A, RTOF_A};
    class AndroidStackMachine {
    public:
        AndroidStackMachine()
            : mainActivity(nullptr),
              activityMap(),
              actionMap(),
              intents(),
              fragments() {}
        AndroidStackMachine(Activity* mainActivity_, 
                            const ActivityMap& activityMap_,
                            const ActionMap& actionMap_,
                            const IntentVec& intents_,
                            const FragmentVec& fragments_)
            : mainActivity(mainActivity_),
              activityMap(activityMap_),
              actionMap(actionMap_),
              intents(intents_),
              fragments(fragments_) {}
        ~AndroidStackMachine() {
            for (auto& pair : activityMap) {
                delete pair.second;
                pair.second = nullptr;
            }
            for (auto intent : intents) {
                delete intent;
                intent = nullptr;
            }
            for (auto fragment : fragments) {
                delete fragment;
                fragment = nullptr;
            }
        }
        Activity* mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode);
        Intent* mkIntent(Activity* activity);
        Fragment* mkFragment(const string& name);
        Activity* getActivity(const string& name) const;
        void setMainActivity(Activity* activity);
        Activity* getMainActivity() const;
        const unordered_set<string>& getAffinities() const;
        const ActivityMap& getActivityMap() const;
        const ActionMap& getActionMap() const;
        void addAction(Activity* activity, Intent* intent);
        void minimize();
        void print() const;
        static ActionMode getMode(Intent* intent);
    private:
        Activity* mainActivity;
        ActivityMap activityMap;
        ActionMap actionMap;
        IntentVec intents;
        FragmentVec fragments;
        unordered_set<string> affinities;
    };
}
#endif /* AndroidStackMachine_hpp */

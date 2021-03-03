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

#include <unordered_map>
#include <unordered_set>
#include "Activity.hpp"
#include "Intent.hpp"
using std::unordered_map, std::unordered_set;
namespace TaskDroid {
    typedef unordered_set<Activity*> Activities;
    typedef unordered_set<Intent*> Intents;
    typedef vector<Fragment*> FragmentVec;
    typedef vector<Activity*> ActivityVec;
    typedef vector<Intent*> IntentVec;
    typedef unordered_map<Activity*, Intents> ActionMap;
    class AndroidStackMachine {
    public:
        AndroidStackMachine()
            : mainActivity(nullptr),
              activities(),
              actionMap(),
              intents(),
              fragments() {}
        AndroidStackMachine(Activity* mainActivity_, 
                            const ActivityVec& activities_,
                            const ActionMap& actionMap_,
                            const IntentVec& intents_,
                            const FragmentVec& fragments_)
            : mainActivity(mainActivity_),
              activities(activities_),
              actionMap(actionMap_),
              intents(intents_),
              fragments(fragments_) {}
        ~AndroidStackMachine() {
            for (auto activity : activities) {
                delete activity;
                activity = nullptr;
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
    private:
        Activity* mainActivity;
        ActivityVec activities;
        ActionMap actionMap;
        IntentVec intents;
        FragmentVec fragments;
    };
}
#endif /* AndroidStackMachine_hpp */

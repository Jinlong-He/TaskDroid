//
//  Activity.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef Activity_hpp 
#define Activity_hpp 

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "FragmentTransaction.hpp"
using std::string, std::unordered_set, std::unordered_map;
namespace TaskDroid {
    enum LaunchMode {STD, STK, STP, SIT};
    typedef unordered_set<FragmentTransaction*> FragmentTransactions;
    typedef vector<FragmentTransaction*> FragmentTransactionVec;
    typedef unordered_map<Fragment*, FragmentTransactions> FragmentTransactionMap;
    class Activity {
    public:
        Activity()
            : name(""),
              affinity(""),
              launchMode(STD),
              fragmentTransactionMap() {}
        Activity(const string& name_, const string& affinity_, const LaunchMode& launchMode_)
            : name(name_),
              affinity(affinity_),
              launchMode(launchMode_) {}
        ~Activity() {
            for (auto fragmentTransaction : fragmentTransactions) {
                delete fragmentTransaction;
                fragmentTransaction = nullptr;
            }
        }
        const string& getName() const;
        void setName(const string& name);
        const string& getAffinity() const;
        void setAffinity(const string& affinity);
        const LaunchMode& getLaunchMode() const;
        void setLaunchMode(const LaunchMode& launchMode);
        const FragmentTransactionMap& getFragmentTransactionMap() const;
        void setFragmentTransactionMap(const FragmentTransactionMap& fragmentTransactionMap);
        void addFragmentTransaction(Fragment* fragment, FragmentTransaction* fragmentTransaction);
        FragmentTransaction* mkFragmentTransaction();
    private:
        string name;
        string affinity;
        LaunchMode launchMode;
        FragmentTransactionMap fragmentTransactionMap;
        FragmentTransactionVec fragmentTransactions;
    };
}

#endif /* Activity_hpp */

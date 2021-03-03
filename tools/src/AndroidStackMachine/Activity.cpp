#include "AndroidStackMachine/Activity.hpp"
namespace TaskDroid {
    const string& Activity::getName() const {
        return name;
    }

    void Activity::setName(const string& name) {
        this -> name = name;
    }

    const string& Activity::getAffinity() const {
        return affinity;
    }

    void Activity::setAffinity(const string& affinity) {
        this -> affinity = affinity;
    }

    const LaunchMode& Activity::getLaunchMode() const {
        return launchMode;
    }

    void Activity::setLaunchMode(const LaunchMode& launchMode) {
        this -> launchMode = launchMode;
    }

    const FragmentTransactionMap& Activity::getFragmentTransactionMap() const {
        return fragmentTransactionMap;
    }

    void Activity::setFragmentTransactionMap(const FragmentTransactionMap& fragmentTransactionMap) {
        this -> fragmentTransactionMap = fragmentTransactionMap;
    }

    void Activity::addFragmentTransaction(Fragment* fragment, FragmentTransaction* fragmentTransaction) {
        fragmentTransactionMap[fragment].insert(fragmentTransaction);
    }

    FragmentTransaction* Activity::mkFragmentTransaction() {
        FragmentTransaction* fragmentTransaction = new FragmentTransaction();
        fragmentTransactions.emplace_back(fragmentTransaction);
        return fragmentTransaction;
    }
}

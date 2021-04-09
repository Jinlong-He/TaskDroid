#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {
    void AndroidStackMachine::setMainActivity(Activity* activity) {
        mainActivity = activity;
    }

    Activity* AndroidStackMachine::getMainActivity() const {
        return mainActivity;
    }

    const AffinityMap& AndroidStackMachine::getAffnityMap() const {
        return affinityMap;
    }

    const ActivityMap& AndroidStackMachine::getActivityMap() const {
        return activityMap;
    }

    const ActionMap& AndroidStackMachine::getActionMap() const {
        return actionMap;
    }

    const FragmentMap& AndroidStackMachine::getFragmentMap() const {
        return fragmentMap;
    }

    ID AndroidStackMachine::getTaskID(const string& affinity) const {
        if (affinityMap.count(affinity) == 0) return -1;
        return affinityMap.at(affinity);
    }

    ID AndroidStackMachine::getViewID(const string& view) const {
        if (viewMap.count(view) == 0) return -1;
        return viewMap.at(view);
    }

    Activity* AndroidStackMachine::mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode) {
        if (activityMap.count(name) == 0) {
            Activity* activity = new Activity(name, affinity, launchMode);
            activityMap[name] = activity;
            if (affinityMap.count(affinity) == 0) {
                affinityMap[affinity] = affinityMap.size();
            }
            return activity;
        }
        return nullptr;
    }

    const FragmentTransactionMap& AndroidStackMachine::getFragmentTransactionMap() const {
        return fragmentTransactionMap;
    }

    const ActivityTransactionMap& AndroidStackMachine::getActivityTransactionMap() const {
        return activityTransactionMap;
    }

    Intent* AndroidStackMachine::mkIntent(Activity* activity) {
        Intent* intent = new Intent(activity);
        intents.emplace_back(intent);
        return intent;
    }

    Fragment* AndroidStackMachine::mkFragment(const string& name) {
        if (fragmentMap.count(name) == 0) {
            Fragment* fragment = new Fragment(name);
            fragmentMap[name] = fragment;
            return fragment;
        }
        return nullptr;
    }

    FragmentTransaction* AndroidStackMachine::mkFragmentTransaction() {
        FragmentTransaction* transaction = new FragmentTransaction();
        fragmentTransactions.emplace_back(transaction);
        return transaction;
    }

    void AndroidStackMachine::addFragmentTransaction(Fragment* fragment,
                                                 FragmentTransaction* transaction) {
        fragmentTransactionMap[fragment].emplace_back(transaction);
    }

    void AndroidStackMachine::addFragmentTransaction(Activity* activity,
                                                 FragmentTransaction* transaction) {
        activityTransactionMap[activity].emplace_back(transaction);
    }

    Activity* AndroidStackMachine::getActivity(const string& name) const {
        if (activityMap.count(name) == 0) return nullptr;
        return activityMap.at(name);
    }

    Fragment* AndroidStackMachine::getFragment(const string& name) const {
        if (fragmentMap.count(name) == 0) return nullptr;
        return fragmentMap.at(name);
    }

    void AndroidStackMachine::addAction(Activity* activity, Intent* intent, bool finish) {
        actionMap[activity].emplace_back(pair(intent, finish));
    }

    void AndroidStackMachine::print() const {
        for (auto& [activity, actions] : actionMap) {
            for (auto& [intent, finish]  : actions) {
                cout << activity -> getName() << "->" << intent -> getActivity() -> getName() << endl;
            }
        }
    }

    void AndroidStackMachine::minimize() {
        affinityMap.clear();
        activityMap.clear();
        for (auto& [source, actions] : actionMap) {
            activityMap[source -> getName()] = source;
            if (source -> getAffinity() != "") {
                if (affinityMap.count(source -> getAffinity()) == 0) {
                    affinityMap[source -> getAffinity()] = affinityMap.size();
                }
            }
            for (auto& [intent, finish] : actions) {
                auto target = intent -> getActivity();
                activityMap[target -> getName()] = target;
                if (target -> getAffinity() != "") {
                    if (affinityMap.count(target -> getAffinity()) == 0) {
                        affinityMap[target -> getAffinity()] = affinityMap.size();
                    }
                }
            }
        }
        for (auto& [activity, transactions] : activityTransactionMap) {
            for (auto& transaction : transactions) {
                for (auto& action : transaction -> getFragmentActions()) {
                    if (viewMap.count(action.getViewID()) == 0) {
                        viewMap[action.getViewID()] = viewMap.size();
                    }
                }
            }
        }
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto& transaction : transactions) {
                for (auto& action : transaction -> getFragmentActions()) {
                    if (viewMap.count(action.getViewID()) == 0) {
                        viewMap[action.getViewID()] = viewMap.size();
                    }
                }
            }
        }
    }

    ActionMode AndroidStackMachine::getMode(Intent* intent) {
        const auto& flags = intent -> getFlags();
        auto activity = intent -> getActivity();
        if (flags.count(F_NTK) > 0) {
            switch (activity -> getLaunchMode()) {
                case STK :
                    if (flags.count(F_CTK) > 0) return CTSK_N;
                    else return CTOP_N;
                    break;
                case STD :
                    if (flags.count(F_CTK) > 0) return CTSK_N;
                    if (flags.count(F_CTP) > 0) return CTOP_N;
                    if (flags.count(F_RTF) > 0) return RTOF_N;
                    if (flags.count(F_STP) > 0) return STOP_N;
                    else return PUSH_N;
                    break;
                case STP :
                    if (flags.count(F_CTK) > 0) return CTSK_N;
                    if (flags.count(F_CTP) > 0) return CTOP_N;
                    if (flags.count(F_RTF) > 0) return RTOF_N;
                    else return STOP_N;
                    break;
                case SIT :
                    break;
            }
        } else {
            switch (activity -> getLaunchMode()) {
                case STK :
                    if (flags.count(F_CTK) > 0) return CTSK_N;
                    else return CTOP_N;
                    break;
                case STD :
                    if (flags.count(F_CTK) > 0) return CTSK;
                    if (flags.count(F_CTP) > 0) return CTOP;
                    if (flags.count(F_RTF) > 0) return RTOF;
                    if (flags.count(F_STP) > 0) return STOP;
                    else return PUSH;
                    break;
                case STP :
                    if (flags.count(F_CTK) > 0) return CTSK;
                    if (flags.count(F_CTP) > 0) return CTOP;
                    if (flags.count(F_RTF) > 0) return RTOF;
                    else return STOP;
                    break;
                case SIT :
                    break;
            }
        }
        return PUSH;
    }

    bool AndroidStackMachine::isNewMode(Intent* intent) {
        return (getMode(intent) > 4);
    }
}

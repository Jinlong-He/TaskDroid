#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {
    void AndroidStackMachine::setPackageName(const string& packageName) {
        this -> packageName = packageName;
    }

    const string& AndroidStackMachine::getPackageName() const {
        return packageName;
    }

    void AndroidStackMachine::setMainActivity(Activity* activity) {
        mainActivity = activity;
    }

    Activity* AndroidStackMachine::getMainActivity() const {
        return mainActivity;
    }

    const AffinityMap& AndroidStackMachine::getAffinityMap() const {
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

    const FragmentMap& AndroidStackMachine::getFragmentMap(Activity* activity) const {
        return activity2FragmentsMap.at(activity);
    }

    const ViewMap& AndroidStackMachine::getViewMap() const {
        return viewMap;
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

    const FragmentTransactionMap& AndroidStackMachine::getFragmentTransactionMap(Activity* activity) const {
        return activity2FragmentTransactionMap.at(activity);
    }

    const ActivityTransactionMap& AndroidStackMachine::getActivityTransactionMap() const {
        return activityTransactionMap;
    }

    const InitFragmentActionMap& AndroidStackMachine::getInitFragmentActionMap(Activity* activity) const {
        return activity2InitFragmentActionMap.at(activity);
    }

    const ViewMap& AndroidStackMachine::getViewMap(Activity* activity) const {
        return activity2ViewMap.at(activity);
    }

    Intent* AndroidStackMachine::mkIntent(Activity* activity) {
        Intent* intent = new Intent(activity);
        intents.insert(intent);
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
        fragmentTransactions.insert(transaction);
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
        bool flag = true;
        if (actionMap.count(activity)) {
            for (auto& [i, f] : actionMap.at(activity)) {
                if (i -> getActivity() == intent -> getActivity() &&
                    i -> getFlags() == intent -> getFlags() &&
                    f == finish) {
                    flag = false;
                    continue;
                }
            }
        }
        if (flag) {
            actionMap[activity].emplace_back(pair(intent, finish));
        }
    }

    void AndroidStackMachine::print(std::ostream& os) const {
        os << "-Package: " << packageName << endl;
        os << "-Main Activity: " << mainActivity -> getName() << endl;
        os << "-Activities: " << endl;
        for (auto& [name, activity] : activityMap) {
            os << name << endl;
        }
        os << "-Activity Transition: " << endl;
        for (auto& [activity, actions] : actionMap) {
            for (auto& [intent, finish]  : actions) {
                os << activity -> getName() << " -> " << intent -> toString();
                if (finish) os << " [finish]";
                os << endl;
            }
        }
        os << "-Fragments: " << endl;
        for (auto& [name, fragment] : fragmentMap) {
            os << name << endl;
        }
        os << "-Fragment Transition: " << endl;
        for (auto& [activity, transactions] : activityTransactionMap) {
            for (auto transaction : transactions) {
                os << activity -> getName() << " -> " << endl
                   << transaction -> toString() << endl;
            }
        }
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto transaction : transactions) {
                os << fragment -> getName() << " -> " << endl 
                   << transaction -> toString() << endl;
            }
        }
    }

    bool AndroidStackMachine::minimize() {
        Activities work({mainActivity}), newWork, acts({mainActivity});
        Fragments fwork, fnewWork, frags;
        while (work.size()) {
            for (auto act : work) {
                if (actionMap.count(act) == 0) continue;
                for (auto& [intent, finish] : actionMap.at(act)) {
                    if (acts.insert(intent -> getActivity()).second)
                        newWork.insert(intent -> getActivity());
                }
            } 
            work.clear();
            if (newWork.size() > 0) work = newWork;
            newWork.clear();
        }
        Activities dels;
        for (auto& [source, actions] : actionMap) {
            if (acts.count(source) == 0) dels.insert(source);
        }
        for (auto del : dels)
            actionMap.erase(del);
        dels.clear();
        for (auto& [activity, transactions] : activityTransactionMap) {
            if (acts.count(activity) == 0) {
                dels.insert(activity);
                continue;
            }
            for (auto transaction : transactions) {
                for (auto action : transaction -> getFragmentActions()) {
                    fwork.insert(action -> getFragment());
                    frags.insert(action -> getFragment());
                }
            }
        }
        for (auto del : dels)
            activityTransactionMap.erase(del);
        dels.clear();
        for (auto& [name, activity] : activityMap) {
            if (acts.count(activity) == 0) dels.insert(activity);
        }
        for (auto del : dels) {
            activityMap.erase(del -> getName());
            delete del;
            del = nullptr;
        }
        affinityMap.clear();
        if (acts.size() == 1 && frags.size() == 0) {
            activityMap.erase(mainActivity -> getName());
            delete mainActivity;
            mainActivity = nullptr;
            setMainActivity(nullptr);
            return false;
        }
        activityMap.clear();
        for (auto activity : acts) {
            activityMap[activity -> getName()] = activity;
            if (affinityMap.count(activity -> getAffinity()) == 0)
                affinityMap[activity -> getAffinity()] = affinityMap.size();
        }

        while (fwork.size()) {
            for (auto fragment : fwork) {
                if (fragmentTransactionMap.count(fragment) == 0) continue;
                for (auto transaction : fragmentTransactionMap.at(fragment)) {
                    for (auto action : transaction -> getFragmentActions()) {
                        if (frags.insert(action -> getFragment()).second)
                            fnewWork.insert(action -> getFragment());
                    }
                }
            } 
            fwork.clear();
            if (fnewWork.size() > 0) fwork = fnewWork;
            fnewWork.clear();
        }
        Fragments fdels;
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            if (frags.count(fragment) == 0) fdels.insert(fragment);
        }
        for (auto del : fdels) fragmentTransactionMap.erase(del);
        fdels.clear();
        for (auto& [name, fragment] : fragmentMap) {
            if (frags.count(fragment) == 0) fdels.insert(fragment);
        }
        for (auto del : fdels) {
            fragmentMap.erase(del -> getName());
            delete del;
            del = nullptr;
        }

        fragmentMap.clear();
        for (auto fragment : frags) 
            fragmentMap[fragment -> getName()] = fragment;
        for (auto& [activity, transactions] : activityTransactionMap) {
            for (auto& transaction : transactions) {
                for (auto& action : transaction -> getFragmentActions()) {
                    if (viewMap.count(action -> getViewID()) == 0) {
                        viewMap[action -> getViewID()] = viewMap.size();
                    }
                }
            }
        }
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto& transaction : transactions) {
                for (auto& action : transaction -> getFragmentActions()) {
                    if (viewMap.count(action -> getViewID()) == 0) {
                        viewMap[action -> getViewID()] = viewMap.size();
                    }
                }
            }
        }
        return true;
    }

    void AndroidStackMachine::fomalize() {
        if (isFomalize) return;
        if (minimize()) formActivity();
        isFomalize = true;
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

    void AndroidStackMachine::formFragmentTransaction(FragmentTransaction* transaction) {
        unordered_map<string, FragmentActions> fragmentActionsMap;
        for (auto action : transaction -> getFragmentActions()) {
            fragmentActionsMap[action -> getViewID()].emplace_back(action);
        }
        transaction -> clear();
        for (auto& [view, actions] : fragmentActionsMap) {
            ID pos = 0;
            for (ID i = 0; i < actions.size(); i++) {
                if (actions[i] -> getFragmentMode() == REP) pos = i;
            }
            vector<Fragment*> fragments;
            auto mode = actions[pos] -> getFragmentMode();
            for (ID i = pos; i < actions.size(); i++) {
                auto fragment = actions[i] -> getFragment();
                auto mode = actions[i] -> getFragmentMode();
                transaction -> addFragmentAction(mode, fragment, view);
                fragments.emplace_back(fragment);
            }
            transaction -> addHighLevelFragmentAction(mode, fragments[fragments.size() - 1], view, fragments);
        }
    }

    void AndroidStackMachine::formActivity(Fragment* fragment, 
                                           FragmentTransactionMap& map,
                                           FragmentMap& fragmentMap) {
        if (fragmentMap.insert(pair(fragment -> getName(), fragment)).second) {
            if (fragmentTransactionMap.count(fragment) == 0) return;
            auto& transactions = fragmentTransactionMap.at(fragment);
            map[fragment] = transactions;
            for (auto& transaction : transactions) {
                for (auto action : transaction -> getFragmentActions()) {
                    formActivity(action -> getFragment(), map, fragmentMap);
                }
            }
        }
    }

    void AndroidStackMachine::formActivity() {
        for (auto& [activity, transactions] : activityTransactionMap) {
            for (auto& transaction : transactions) {
                formFragmentTransaction(transaction);
            }
        }
        for (auto& [fragment, transactions] : fragmentTransactionMap) {
            for (auto& transaction : transactions) {
                formFragmentTransaction(transaction);
            }
        }
        for (auto& [activity, transactions] : activityTransactionMap) {
            auto& initFragmentActionMap = activity2InitFragmentActionMap[activity];
            auto& fragmentMap = activity2FragmentsMap[activity];
            auto& fragmentTransactionMap = 
                activity2FragmentTransactionMap[activity];
            auto& viewMap = activity2ViewMap[activity];
            for (auto transaction : transactions) {
                auto& initMap = initFragmentActionMap[transaction];
                for (auto action : transaction -> getHighLevelFragmentActions()) {
                    auto viewID = getViewID(action -> getViewID());
                    initMap[viewID] = action;
                }
                for (auto action : transaction -> getFragmentActions()) {
                    auto fragment = action -> getFragment();
                    auto viewID = getViewID(action -> getViewID());
                    formActivity(fragment, fragmentTransactionMap, fragmentMap);
                }
            }
            for (auto& [fragment, transactions] : fragmentTransactionMap) {
                for (auto& transaction : transactions) {
                    for (auto& action : transaction -> getFragmentActions()) {
                        if (viewMap.count(action -> getViewID()) == 0) {
                            viewMap[action -> getViewID()] = viewMap.size();
                        }
                    }
                }
            }
        }
    }
}

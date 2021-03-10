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

    const unordered_set<string>& AndroidStackMachine::getAffinities() const {
        return affinities;
    }

    const ActivityMap& AndroidStackMachine::getActivityMap() const {
        return activityMap;
    }

    const ActionMap& AndroidStackMachine::getActionMap() const {
        return actionMap;
    }

    Activity* AndroidStackMachine::mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode) {
        if (activityMap.count(name) == 0) {
            Activity* activity = new Activity(name, affinity, launchMode);
            activityMap[name] = activity;
            return activity;
        }
        return nullptr;
    }

    Intent* AndroidStackMachine::mkIntent(Activity* activity) {
        Intent* intent = new Intent(activity);
        intents.emplace_back(intent);
        return intent;
    }

    Activity* AndroidStackMachine::getActivity(const string& name) const {
        if (activityMap.count(name) == 0) return nullptr;
        return activityMap.at(name);
    }

    void AndroidStackMachine::addAction(Activity* activity, Intent* intent) {
        actionMap[activity].insert(intent);
    }

    void AndroidStackMachine::print() const {
        for (auto& pair : actionMap) {
            for (auto intent : pair.second) {
                cout << pair.first -> getName() << "->" << intent -> getActivity() -> getName() << endl;
            }
        }
    }

    void AndroidStackMachine::minimize() {
        affinities.clear();
        activityMap.clear();
        for (auto& [source, intents] : actionMap) {
            activityMap[source -> getName()] = source;
            if (source -> getAffinity() != "") affinities.insert(source -> getAffinity());
            for (auto& intent : intents) {
                auto target = intent -> getActivity();
                activityMap[target -> getName()] = target;
                if (target -> getAffinity() != "") affinities.insert(target -> getAffinity());
            }
        }
    }

    ActionMode AndroidStackMachine::getMode(Intent* intent) {
        const auto& flags = intent -> getFlags();
        auto activity = intent -> getActivity();
        if (flags.count(F_NTK) > 0) {
            switch (activity -> getLaunchMode()) {
                case STK :
                    if (flags.count(F_CTK) > 0) return CTSK_A;
                    else return CTOP_A;
                    break;
                case STD :
                    if (flags.count(F_CTK) > 0) return CTSK_A;
                    if (flags.count(F_CTP) > 0) return CTOP_A;
                    if (flags.count(F_RTF) > 0) return RTOF_A;
                    if (flags.count(F_STP) > 0) return STOP_A;
                    else return PUSH_A;
                    break;
                case STP :
                    if (flags.count(F_CTK) > 0) return CTSK_A;
                    if (flags.count(F_CTP) > 0) return CTOP_A;
                    if (flags.count(F_RTF) > 0) return RTOF_A;
                    else return STOP_A;
                    break;
                case SIT :
                    break;
            }
        } else {
            switch (activity -> getLaunchMode()) {
                case STK :
                    if (flags.count(F_CTK) > 0) return CTSK_A;
                    else return CTOP_A;
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
}

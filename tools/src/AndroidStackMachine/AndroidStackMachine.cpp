#include "AndroidStackMachine/AndroidStackMachine.hpp"
namespace TaskDroid {
    Activity* AndroidStackMachine::mkActivity(const string& name, const string& affinity, const LaunchMode& launchMode) {
        Activity* activity = new Activity(name, affinity, launchMode);
        activities.emplace_back(activity);
        return activity;
    }

    Intent* AndroidStackMachine::mkIntent(Activity* activity) {
        Intent* intent = new Intent(activity);
        intents.emplace_back(intent);
        return intent;
    }
}

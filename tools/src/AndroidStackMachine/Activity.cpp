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
}

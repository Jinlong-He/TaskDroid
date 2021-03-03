#include "AndroidStackMachine/Intent.hpp"
namespace TaskDroid {
    Activity* Intent::getActivity() const {
        return activity;
    }

    void Intent::setActivity(Activity* activity) {
        this -> activity = activity;
    }

    const FLAGs& Intent::getFlags() const {
        return flags;
    }

    void Intent::setFlags(const FLAGs& flags) {
        this -> flags = flags;
    }

    void Intent::addFlag(const FLAG& flag) {
        this -> flags.emplace_back(flag);
    }
}

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
        this -> flags.insert(flag);
    }

    void Intent::addFlag(const string& flag) {
        if (flag == "FLAG_ACTIVITY_NEW_TASK") {
            this -> flags.insert(F_NTK);
        } else if (flag == "FLAG_ACTIVITY_CLEAR_TASK") {
            this -> flags.insert(F_CTK);
        } else if (flag == "FLAG_ACTIVITY_CLEAR_TOP") {
            this -> flags.insert(F_CTP);
        } else if (flag == "FLAG_ACTIVITY_SINGLE_TOP") {
            this -> flags.insert(F_STP);
        } else if (flag == "FLAG_ACTIVITY_MULTIPLE_TASK") {
            this -> flags.insert(F_MTK);
        } else if (flag == "FLAG_ACTIVITY_TASK_ON_HOME") {
            this -> flags.insert(F_TOM);
        } else if (flag == "FLAG_ACTIVITY_REORDER_TO_FRONT") {
            this -> flags.insert(F_RTF);
        }
    }
}

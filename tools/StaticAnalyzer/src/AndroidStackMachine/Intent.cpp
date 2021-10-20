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

    string Intent::toString() const {
        if (flags.size() == 0) return activity -> getName();
        string res = activity -> getName() + " (";
        for (auto flag : flags) {
            switch (flag) {
                case F_NTK :
                    res += "FLAG_ACTIVITY_NEW_TASK,";
                    break;
                case F_CTK :
                    res += "FLAG_ACTIVITY_CLEAR_TASK,";
                    break;
                case F_CTP :
                    res += "FLAG_ACTIVITY_CLEAR_TOP,";
                    break;
                case F_STP :
                    res += "FLAG_ACTIVITY_SINGLE_TOP,";
                    break;
                case F_MTK :
                    res += "FLAG_ACTIVITY_MULTIPLE_TASK,";
                    break;
                case F_TOM :
                    res += "FLAG_ACTIVITY_TASK_ON_HOME,";
                    break;
                case F_RTF :
                    res += "FLAG_ACTIVITY_REORDER_TO_FRONT,";
                    break;
            }
        }
        res = res.substr(0, res.length() - 1);
        return res + ")";
    }
}

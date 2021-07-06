//
//  Intent.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef Intent_hpp 
#define Intent_hpp 

#include <vector>
#include "Activity.hpp"
using std::vector;
namespace TaskDroid {
    enum FLAG {F_NTK, F_CTP, F_STP, F_CTK, F_MTK, F_TOM, F_RTF};
    typedef unordered_set<FLAG> FLAGs;
    class Intent {
    public:
        Intent()
            : activity(nullptr),
              flags() {}
        Intent(Activity* activity_, const FLAGs& flags_ = FLAGs())
            : activity(activity_),
              flags(flags_) {}
        ~Intent() {}
        Activity* getActivity() const;
        void setActivity(Activity* activity);
        const FLAGs& getFlags() const;
        void setFlags(const FLAGs& flags);
        void addFlag(const FLAG& flag);
        void addFlag(const string& flag);
        string toString() const;
    private:
        Activity* activity;
        FLAGs flags;
    };
}

#endif /* Intent_hpp */

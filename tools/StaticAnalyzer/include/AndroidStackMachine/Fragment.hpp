//
//  Fragment.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef Fragment_hpp 
#define Fragment_hpp 

#include <string>
using std::string;
namespace TaskDroid {
    class Fragment {
    public:
        Fragment()
            : name("") {}
        Fragment(const string& name_)
            : name(name_) {}
        ~Fragment() {}
        const string& getName() const;
        void setName(const string& name);
    private:
        string name;
    };
}
#endif /* Fragment_hpp */

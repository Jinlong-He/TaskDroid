//
//  Configuration.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef Configuration_hpp 
#define Configuration_hpp 

#include <string>
#include <vector>
#include "Activity.hpp"
using std::string, std::vector, std::pair;
namespace TaskDroid {
    template <class Symbol>
    class Configuration {
    public:
        Configuration() {}
        void addTask(const string& address, const vector<Symbol*> task);
    private:
        vector<pair<string, vector<Symbol*> > > content;
    };
}
#endif /* Configuration_hpp */

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
        typedef pair<string, vector<Symbol*> > TaskConfigration;
        Configuration() {}
        void addTask(const string& address, const vector<Symbol*> task) {
            content.emplace_back(pair(address, task));
        }
        const vector<TaskConfigration>& getContent() const {
            return content;
        }
    private:
        vector<TaskConfigration> content;
    };
}
#endif /* Configuration_hpp */

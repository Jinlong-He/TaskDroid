#include "AndroidStackMachine/Configuration.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {
    void Configuration::addTask(const string& affinity, 
                                const vector<Activity*> task) {
        content.push_back(pair(affinity, task));
    }
}

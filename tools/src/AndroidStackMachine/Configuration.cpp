#include "AndroidStackMachine/Configuration.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {
    template <class Symbol>
    void Configuration<Symbol>::addTask(const string& address, 
                                        const vector<Symbol*> task) {
        content.push_back(pair(address, task));
    }
}

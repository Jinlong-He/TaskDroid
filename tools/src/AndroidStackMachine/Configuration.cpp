#include "AndroidStackMachine/Configuration.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {

    template <class Symbol>
    const vector<typename Configuration<Symbol>::TaskConfigration>& Configuration<Symbol>::getContent() const {
        return content;
    }

    template <class Symbol>
    void Configuration<Symbol>::addTask(const string& address, const vector<Symbol*> task) {
        content.emplace_back(pair(address, task));
    }
}

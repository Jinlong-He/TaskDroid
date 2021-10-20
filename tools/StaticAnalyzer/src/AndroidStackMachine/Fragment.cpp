#include "AndroidStackMachine/Fragment.hpp"
namespace TaskDroid {
    const string& Fragment::getName() const {
        return name;
    }

    void Fragment::setName(const string& name) {
        this -> name = name;
    }
}

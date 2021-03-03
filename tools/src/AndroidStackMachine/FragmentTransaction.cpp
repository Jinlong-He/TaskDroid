#include "AndroidStackMachine/FragmentTransaction.hpp"
namespace TaskDroid {
    bool FragmentTransaction::isAddTobackStack() const {
        return addToBackStack;
    }

    void FragmentTransaction::setAddTobackStack(bool addToBackStack) {
        this -> addToBackStack = addToBackStack;
    }

    const FragmentActions& FragmentTransaction::getFragmentActions() const {
        return fragmentActions;
    }

    void FragmentTransaction::setFragmentActions(const FragmentActions& fragmentActions) {
        this -> fragmentActions = fragmentActions;
    }

    void FragmentTransaction::addFragmentAction(Action action, Fragment* fragment, ID viewID) {
        fragmentActions.emplace_back(FragmentTransaction::FragmentAction(action, fragment, viewID));
    }
}

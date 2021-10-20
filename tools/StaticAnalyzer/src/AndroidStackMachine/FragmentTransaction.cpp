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

    const FragmentActions& FragmentTransaction::getHighLevelFragmentActions() const {
        return highlevelFragmentActions;
    }

    void FragmentTransaction::setFragmentActions(const FragmentActions& fragmentActions) {
        this -> fragmentActions = fragmentActions;
    }

    void FragmentTransaction::addFragmentAction(FragmentMode mode, Fragment* fragment, const string viewID) {
        fragmentActions.emplace_back(new FragmentAction(mode, fragment, viewID));
    }

    void FragmentTransaction::addHighLevelFragmentAction(FragmentMode mode, Fragment* fragment, const string viewID, const vector<Fragment*>& fragments) {
        highlevelFragmentActions.emplace_back(new FragmentAction(mode, fragment, viewID, fragments));
    }

    void FragmentTransaction::clear() {
        fragmentActions.clear();
    }

    string FragmentTransaction::toString() const {
        string res = "";
        for (auto action : fragmentActions) {
            res += action -> toString() + "\n";
        }
        if (isAddTobackStack()) res += "addToBackStack()\n";
        res += "commit()";
        return res;
    }
}

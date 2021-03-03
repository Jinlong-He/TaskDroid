#include <iostream>
#include "AndroidStackMachine/AndroidStackMachine.hpp"
using namespace TaskDroid;
using std::cout, std::endl;
int main () {
    AndroidStackMachine a;
    cout << a.mkActivity("A", "1", STD) -> getName() << endl;
    return 0;
}

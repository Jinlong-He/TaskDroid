#include <iostream>
#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include "Parser/ASMParser.hpp"
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "Analyzer/FragmentAnalyzer.hpp"
using namespace TaskDroid;
using std::cout, std::endl;
int main (int argc, char* argv[]) {
    AndroidStackMachine a;
    auto home = a.mkFragment("home");
    auto ft = a.mkFragmentTransaction();
    ft -> addFragmentAction(ADD, home, "1");
    ft -> setAddTobackStack(1);
    a.addFragmentTransaction(home, ft);
    FragmentAnalyzer analyzer(2,2);
    analyzer.loadASM(&a);
    analyzer.analyzeReachability();
    //ASMParser::parseManifest(argv[1], &a);
    //ASMParser::parseFragment(argv[2], &a);
    //ASMParser::parseATG(argv[2], &a);
    //MultiTaskAnalyzer::analyzeReachability(&a, 5);
    //MultiTaskAnalyzer analyzer(6);
    //analyzer.loadASM(&a);
    //analyzer.analyzeBoundedness();
    return 0;
}

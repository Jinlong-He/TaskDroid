#include <iostream>
#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include "Parser/ASMParser.hpp"
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "Analyzer/FragmentAnalyzer.hpp"
using namespace TaskDroid;
using std::cout, std::endl;
int main (int argc, char* argv[]) {
    AndroidStackMachine a;
    ASMParser::parseManifest(argv[1], &a);
    ASMParser::parseATG(argv[2], &a);
    ASMParser::parseFragment(argv[3], &a);
    FragmentAnalyzer analyzer(5,2);
    analyzer.loadASM(&a);
    for (auto& [name, activity] : a.getActivityMap()) {
        if (a.getActivityTransactionMap().count(activity) != 0) {
            if (a.getFragmentTransactionMap(activity).size() > 0) {
                vector<Fragment*> task({a.getFragment("CFragment"),
                                        a.getFragment("BFragment"),
                                        a.getFragment("AFragment"),
                                        a.getFragment("CFragment"),
                                        a.getFragment("BFragment"),
                                        a.getFragment("DFragment")});
                //vector<Fragment*> task({a.getFragment("CFragment")});
                analyzer.analyzeReachability(activity, "16908290", task);
                break;
            }
        }
    }
    //ASMParser::parseATG(argv[2], &a);
    //MultiTaskAnalyzer::analyzeReachability(&a, 5);
    //MultiTaskAnalyzer analyzer(6);
    //analyzer.loadASM(&a);
    //analyzer.analyzeBoundedness();
    return 0;
}

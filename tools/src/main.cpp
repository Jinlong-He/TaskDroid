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
    a.fomalize();
    bool flag = true;
    std::ofstream out(argv[4]);
    for (auto& [name, activity] : a.getActivityMap()) {
        if (a.getActivityTransactionMap().count(activity) != 0) {
            if (a.getFragmentTransactionMap(activity).size() > 0) {
                flag = false;
                FragmentAnalyzer analyzer(2,3,&a,activity);
                analyzer.analyzeBoundedness(out);
            }
        }
    }
    if (flag) {
        out << "No Fragments" << endl;
    }
    if (!flag) {
        out << "Has Fragments" << endl;
    }
    //ASMParser::parseATG(argv[2], &a);
    //MultiTaskAnalyzer::analyzeReachability(&a, 5);
    //MultiTaskAnalyzer analyzer(6);
    //analyzer.loadASM(&a);
    //analyzer.analyzeBoundedness();
    return 0;
}

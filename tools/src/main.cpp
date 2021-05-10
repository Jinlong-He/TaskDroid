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
    Configuration<Fragment> config;
    a.fomalize();
    for (auto& [name, activity] : a.getActivityMap()) {
        if (a.getActivityTransactionMap().count(activity) != 0) {
            if (a.getFragmentTransactionMap(activity).size() > 0) {
                FragmentAnalyzer analyzer(2,3,&a,activity);
                std::ofstream out(argv[4]);
                analyzer.analyzeBoundedness(out);
                //ASMParser::parseFragmentConfig(argv[4], &a, &config);
                //auto& content = config.getContent();
                //analyzer.analyzeReachability(activity, content[0].first, content[0].second);
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

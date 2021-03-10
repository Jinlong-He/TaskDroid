#include <iostream>
#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include "Parser/ASMParser.hpp"
#include "Analyzer/MutiTaskAnalyzer.hpp"
using namespace TaskDroid;
using std::cout, std::endl;
int main (int argc, char* argv[]) {
    AndroidStackMachine a;
    ASMParser::parseManifest(argv[1], &a);
    ASMParser::parseATG(argv[2], &a);
    MutiTaskAnalyzer::analyzeReachability(&a, 5);
    return 0;
}

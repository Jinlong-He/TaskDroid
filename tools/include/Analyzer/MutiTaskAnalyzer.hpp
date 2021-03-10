//
//  MutiTaskAnalyzer.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef MutiTaskAnalyzer_hpp 
#define MutiTaskAnalyzer_hpp 

#include "../AndroidStackMachine/AndroidStackMachine.hpp"
namespace TaskDroid {
    class MutiTaskAnalyzer {
    public:
        static void analyze(AndroidStackMachine* a);
        static void analyzeBoundedness(AndroidStackMachine* a);
        static void analyzeReachability(AndroidStackMachine* a, int k);
    };
}
#endif /* MutiTaskAnalyzer_hpp */

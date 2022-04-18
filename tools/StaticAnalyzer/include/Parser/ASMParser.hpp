//
//  ASMParser.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef ASMParser_hpp 
#define ASMParser_hpp 

#include "tinyxml2.h"
#include "../AndroidStackMachine/AndroidStackMachine.hpp"
using namespace tinyxml2;
namespace TaskDroid {
    class ASMParser {
    public:
        static void parse(const char* fileName, AndroidStackMachine* a);
        static void parseManifest(const char* fileName, AndroidStackMachine* a, bool amm = true);
        static void parseManifestTxt(const char* fileName, AndroidStackMachine* a, bool amm = true);
        static void parseATG(const char* fileName, AndroidStackMachine* a, const char* gator = nullptr);
        static void parseFragment(const char* fileName, AndroidStackMachine* a, bool amm = true);
        static void parseFragmentConfig(const char* fileName, 
                                        AndroidStackMachine* a,
                                        Configuration<Fragment>* config);
    };
}
#endif /* ASMParser_hpp */

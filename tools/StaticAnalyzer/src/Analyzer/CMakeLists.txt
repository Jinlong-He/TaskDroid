
MESSAGE(STATUS "Model This is SOURCE dir " ${PROJECT_SOURCE_DIR})

ADD_LIBRARY(Analyzer MultiTaskAnalyzer.cpp
                     LegalPosMaker.cpp
                     OrderMaker.cpp
                     ValuesMaker.cpp
                     NuXmvTranslator.cpp
                     MultiTaskSemanticsAnalyzer.cpp
                     MultiTaskBoundednessAnalyzer.cpp
                     MultiTaskUnexpectednessAnalyzer.cpp
                     FragmentAnalyzer.cpp)



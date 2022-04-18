
MESSAGE(STATUS "Model This is SOURCE dir " ${PROJECT_SOURCE_DIR})

ADD_LIBRARY(Analyzer MultiTaskReachabilityAnalyzer.cpp
                     MultiTaskRealActivityAnalyzer.cpp
                     MultiTaskBoundednessAnalyzer.cpp
                     MultiTaskBackHijacking.cpp
                     MultiTaskTestTraceGenerator.cpp
                     MultiTaskUBBP.cpp
                     FragmentAnalyzer.cpp
                     FragmentUBBP.cpp)


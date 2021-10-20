MESSAGE(STATUS "src This is SOURCE dir " ${PROJECT_SOURCE_DIR})

ADD_SUBDIRECTORY(AndroidStackMachine)
ADD_SUBDIRECTORY(Parser)
ADD_SUBDIRECTORY(Analyzer)
ADD_EXECUTABLE(StaticAnalyzer main.cpp)
TARGET_LINK_LIBRARIES(StaticAnalyzer AndroidStackMachine 
                                     Parser 
                                     Analyzer
                                     -lboost_program_options)

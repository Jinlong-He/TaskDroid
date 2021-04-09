MESSAGE(STATUS "Model This is SOURCE dir " ${PROJECT_SOURCE_DIR})

ADD_LIBRARY(AndroidStackMachine Fragment.cpp 
                                FragmentTransaction.cpp 
                                Activity.cpp 
                                Intent.cpp 
                                Configuration.cpp
                                AndroidStackMachine.cpp)
    


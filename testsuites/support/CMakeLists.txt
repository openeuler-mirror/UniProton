add_library(testsuite_support OBJECT main.c)
target_include_directories(testsuite_support INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
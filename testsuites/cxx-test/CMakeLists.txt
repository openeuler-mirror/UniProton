file(GLOB SRC_LIST
    ./misc/*.cc
    ./run_test.cc
)
add_library(cxxtest OBJECT ${SRC_LIST})
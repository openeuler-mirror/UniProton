include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../util 
)

file(GLOB TEST_UTIL ${CMAKE_CURRENT_SOURCE_DIR}/../util/*.cc)
file(GLOB TEST_BOOST ${CMAKE_CURRENT_SOURCE_DIR}/../boost/system/*.cc)

execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_helper.py
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.compiles_list
    ERROR_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.compiles_list
)

file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/.compiles_list SRC_LIST)

set(ALL_SRC ${TEST_UTIL} ${TEST_BOOST} ${SRC_LIST} .run_test.cc)

add_library(cxxtest OBJECT ${ALL_SRC})




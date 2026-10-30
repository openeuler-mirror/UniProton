set(TSID_DIR ${HOME_PATH}/demos/sd3403/component/tsid-1.7.1)
set(EQ_DIR ${HOME_PATH}/demos/sd3403/component/eiquadprog-1.3.2)
file(GLOB TSID_SRCS ${TSID_DIR}/src/*/*.cpp)
# optional solver backends whose dependencies are not installed on bare metal
list(FILTER TSID_SRCS EXCLUDE REGEX "qpmad|osqp")
set(ALL_SRC tsid_test.cc ${EQ_DIR}/src/eiquadprog-fast.cpp ${EQ_DIR}/src/eiquadprog.cpp ${TSID_SRCS}
    ${HOME_PATH}/demos/sd3403/apps/openamp/openarm_gravity.c)
set_source_files_properties(${ALL_SRC} PROPERTIES COMPILE_OPTIONS "-O2")
add_library(tsidtest OBJECT ${ALL_SRC})
target_include_directories(tsidtest PUBLIC ${TSID_DIR}/include ${EQ_DIR}/include)

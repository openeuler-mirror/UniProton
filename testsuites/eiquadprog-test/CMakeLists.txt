set(EQ_DIR ${HOME_PATH}/demos/sd3403/component/eiquadprog-1.3.2)
set(ALL_SRC
    eiquadprog_test.cc
    ${EQ_DIR}/src/eiquadprog-fast.cpp
)
set_source_files_properties(${ALL_SRC} PROPERTIES COMPILE_OPTIONS "-O2")
add_library(eqtest OBJECT ${ALL_SRC})
target_include_directories(eqtest PUBLIC ${EQ_DIR}/include)

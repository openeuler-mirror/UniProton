if (NOT ${APP} STREQUAL "UniProton_soem_demo")
    return()
endif()

set(BUILD_APP "UniProton_soem_demo")
set(ALL_SRC
    soem_demo.c
    test/linux/simple_test/simple_test.c
    test/linux/slaveinfo/slaveinfo.c
)

if(${CPU_TYPE} STREQUAL "hi3093")
    add_library(soemTest OBJECT ${ALL_SRC})
endif()
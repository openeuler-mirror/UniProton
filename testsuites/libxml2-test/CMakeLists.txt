if (NOT ${APP} STREQUAL "UniPorton_test_libxml2_interface")
    return()
endif()

set(BUILD_APP "UniPorton_test_libxml2_interface")
set(ALL_SRC xmltest.c)

if(${CPU_TYPE} STREQUAL "x86_64")
    add_library(libxml2Test OBJECT ${ALL_SRC})
endif()
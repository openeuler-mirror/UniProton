if (NOT ${APP} STREQUAL "linuxTest")
    return()
endif()

set(BUILD_APP "linuxTest")
file(GLOB_RECURSE LINUX_TEST_SOURCE
    *.c
)

add_library(ighTest OBJECT ${LINUX_TEST_SOURCE})
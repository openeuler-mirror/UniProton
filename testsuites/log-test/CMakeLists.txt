if (NOT ${APP} STREQUAL "UniPorton_test_log_interface")
    return()
endif()

set(BUILD_APP "UniPorton_test_log_interface")
set(ALL_SRC log_test.c)

add_library(logTest OBJECT ${ALL_SRC})
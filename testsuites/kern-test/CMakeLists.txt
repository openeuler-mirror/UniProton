if (NOT ${APP} STREQUAL "UniPorton_test_sem")
    return()
endif()

set(BUILD_APP "UniPorton_test_sem")
set(ALL_SRC sem_mutex_test.c)

add_library(semTest OBJECT ${ALL_SRC})
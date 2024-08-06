if (${APP} STREQUAL "task-switch" OR ${APP} STREQUAL "task-preempt" OR
    ${APP} STREQUAL "semaphore-shuffle" OR ${APP} STREQUAL "interrupt-latency" OR
    ${APP} STREQUAL "deadlock-break" OR ${APP} STREQUAL "message-latency")
    set (SRC main.c rpmsg_backend.c rpmsg_service.c example_default.c)
elseif (${APP} STREQUAL "ethercatTest")
    set (SRC main.c rpmsg_backend.c rpmsg_service.c example_default.c demo.c)
else()
    set (SRC main.c rpmsg_backend.c rpmsg_service.c example_default.c)
endif()
add_library(ethercat OBJECT ${SRC})

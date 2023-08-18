if (${APP} STREQUAL "task-switch" OR ${APP} STREQUAL "task-preempt" OR
    ${APP} STREQUAL "semaphore-shuffle" OR ${APP} STREQUAL "interrupt-latency" OR
    ${APP} STREQUAL "deadlock-break" OR ${APP} STREQUAL "message-latency")
    set (SRC rpmsg_backend.c rpmsg_service.c)
else()
    set (SRC ethercat_igh.c main.c rpmsg_backend.c rpmsg_service.c)
endif()
add_library(ethercat OBJECT ${SRC})

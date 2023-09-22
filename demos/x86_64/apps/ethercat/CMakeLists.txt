if (RPC_DEMO_SWITCH)
add_definitions(-DSTATIC_EP)
set (SRC ethercat_igh.c main.c rpmsg_backend.c rpmsg_service.c example_rpc.c)
elseif (${APP} STREQUAL "task-switch" OR ${APP} STREQUAL "task-preempt" OR
    ${APP} STREQUAL "semaphore-shuffle" OR ${APP} STREQUAL "interrupt-latency" OR
    ${APP} STREQUAL "deadlock-break" OR ${APP} STREQUAL "message-latency")
    set (SRC rpmsg_backend.c rpmsg_service.c example_default.c)
else()
    set (SRC ethercat_igh.c main.c rpmsg_backend.c rpmsg_service.c example_default.c demo.c)
endif()
add_library(ethercat OBJECT ${SRC})
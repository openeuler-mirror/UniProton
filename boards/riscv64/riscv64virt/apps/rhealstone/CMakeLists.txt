# if our rhealstone directory changed you can change this option to adapt it
# and be careful, we don't need rhealstone has a fixed path
# but we need to make sure the structure of rhealstone directory
set(RHEAL_TST_PATH "${HOME_PATH}/testsuites/rhealstone")

if (${APP} STREQUAL "deadlock-break")
    set(BUILD_APP "deadlock-break")
    set(ALL_SRC  ${RHEAL_TST_PATH}/deadlock-break/deadlock-break.c)
elseif(${APP} STREQUAL "interrupt-latency")
    set(BUILD_APP "interrupt-latency")
    set(ALL_SRC  ${RHEAL_TST_PATH}/interrupt-latency/interrupt-latency.c)
elseif(${APP} STREQUAL "message-latency")
    set(BUILD_APP "message-latency")
    set(ALL_SRC  ${RHEAL_TST_PATH}/message-latency/message-latency.c)
elseif(${APP} STREQUAL "semaphore-shuffle")
    set(BUILD_APP "semaphore-shuffle")
    set(ALL_SRC  ${RHEAL_TST_PATH}/semaphore-shuffle/semaphore-shuffle.c)
elseif(${APP} STREQUAL "task-preempt")
    set(BUILD_APP "task-preempt")
    set(ALL_SRC  ${RHEAL_TST_PATH}/task-preempt/task-preempt.c)
elseif(${APP} STREQUAL "task-switch")
    set(BUILD_APP "task-switch")
    set(ALL_SRC  ${RHEAL_TST_PATH}/task-switch/task-switch.c)
else()
    return()
endif()

add_library(app OBJECT main.c ${ALL_SRC} ${RHEAL_TST_PATH}/support/banchmark_support.c)
target_include_directories(app PUBLIC ${RHEAL_TST_PATH}/support)

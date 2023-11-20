add_subdirectory(support)

if (${APP} STREQUAL "deadlock-break")
    set(BUILD_APP "deadlock-break")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/deadlock-break/deadlock-break.c)
elseif(${APP} STREQUAL "interrupt-latency")
    set(BUILD_APP "interrupt-latency")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/interrupt-latency/interrupt-latency.c)
elseif(${APP} STREQUAL "message-latency")
    set(BUILD_APP "message-latency")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/message-latency/message-latency.c)
elseif(${APP} STREQUAL "semaphore-shuffle")
    set(BUILD_APP "semaphore-shuffle")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/semaphore-shuffle/semaphore-shuffle.c)
elseif(${APP} STREQUAL "task-preempt")
    set(BUILD_APP "task-preempt")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/task-preempt/task-preempt.c)
elseif(${APP} STREQUAL "task-switch")
    set(BUILD_APP "task-switch")
    set(ALL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/task-switch/task-switch.c)
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "m4")
list(APPEND OBJS 
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
)

add_executable(${BUILD_APP} ${ALL_SRC} ${CXX_LIB} ${OBJS})
target_link_libraries(${BUILD_APP} PUBLIC testsuite_support rhealstone_support)
endif()

if (${CPU_TYPE} STREQUAL "rk3568_jailhouse" OR ${CPU_TYPE} STREQUAL "x86_64" OR ${CPU_TYPE} STREQUAL "armv8")
    add_library(rhealstoneTest OBJECT ${ALL_SRC} ./support/banchmark_support.c)
    target_include_directories(rhealstoneTest PUBLIC ./support)
endif()

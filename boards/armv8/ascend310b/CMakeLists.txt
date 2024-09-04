include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

if (${CONFIG_OS_OPTION_OPENAMP})
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp/ascend310b
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
        ${HOME_PATH}/src/include/uapi/
        ${HOME_PATH}/platform/libboundscheck/include
        ${HOME_PATH}/src/arch/include
        ${HOME_PATH}/src/core/kernel/include
        ${HOME_PATH}/src/utility/lib/include
        ${HOME_PATH}/src/shell/adapter/include
        ${HOME_PATH}/src/shell/full/include
        ${HOME_PATH}/src/component/mica
        ${HOME_PATH}/src/component/proxy
        ${HOME_PATH}/src/om/include
        ${HOME_PATH}/src/mem/include
        ${HOME_PATH}/build/open-amp/output/usr/local/include
        ${HOME_PATH}/build/libmetal/output/usr/local/include
        ${HOME_PATH}/src/libc/musl/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/build/output/drivers/include/
    )
elseif(${CONFIG_GUEST_OS})
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp/ascend310b
        ${HOME_PATH}/src/arch/include
        ${HOME_PATH}/src/utility/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/helloworld
    )
endif()

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
list(APPEND OBJS $<TARGET_OBJECTS:proxy>)

if (${CONFIG_OS_OPTION_OPENAMP})
    add_subdirectory(${HOME_PATH}/src/component/mica mica)
    list(APPEND OBJS $<TARGET_OBJECTS:mica>)
    if (${APP} STREQUAL "UniPorton_test_posix_time_interface" OR 
        ${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface" OR
        ${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface" OR
        ${APP} STREQUAL "UniPorton_test_posix_malloc_interface" OR
        ${APP} STREQUAL "UniPorton_test_posix_signal_interface")
            add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
            target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
            # 如果从核需要跑测试用例，config加上测试用例选项
            # target_compile_options(config PUBLIC -DPOSIX_TESTCASE)
            list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
    elseif (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
    elseif (${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
        add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
        target_compile_options(rpmsg PUBLIC -DRHEALSTONE_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:rhealstoneTest>)
    elseif(${APP} STREQUAL "UniPorton_test_log_interface")
        add_subdirectory(${HOME_PATH}/testsuites/log-test tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
        target_compile_options(config PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:logTest>)
    else()
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
    endif()
elseif(${CONFIG_GUEST_OS})
    list(APPEND OBJS $<TARGET_OBJECTS:apps> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
endif()

add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})

include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/hi3093
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${HOME_PATH}/build/libcxx/include
    ${HOME_PATH}/build/output/soem/include
    ${HOME_PATH}/src/component/boost_1_54_0
    ${HOME_PATH}/src/component/eigen-3.4.0
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/component/mica
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/include
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
if (${APP} STREQUAL "cxxTest" OR ${APP} STREQUAL "eigenTest")
add_subdirectory(${HOME_PATH}/src/component/boost_1_54_0 boost)
endif()
if(${CONFIG_OS_SUPPORT_LIBXML2})
    add_subdirectory(component/libxml2)
endif()

if (${CONFIG_OS_OPTION_OPENAMP})
    add_subdirectory(${HOME_PATH}/src/component/mica mica)
    list(APPEND OBJS $<TARGET_OBJECTS:mica>)
endif()

if (${APP} STREQUAL "UniPorton_test_posix_time_interface" OR 
    ${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_malloc_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_signal_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        # 如果从核需要跑测试用例，config加上测试用例选项
        # target_compile_options(config PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
elseif (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
elseif(${APP} STREQUAL "cxxTest")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/cxx-test tmp)
        target_compile_options(rpmsg PUBLIC -DCXX_TESTCASE)
        target_compile_options(cxxtest PUBLIC -D_POSIX_THREADS -DBOOST_HAS_PTHREAD_MUTEXATTR_SETTYPE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:cxxtest>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_CXX AND CONFIG_OS_OPTION_LOCALE")
        return()
    endif()
elseif(${APP} STREQUAL "eigenTest")
    add_subdirectory(${HOME_PATH}/testsuites/eigen-test tmp)
    target_compile_options(rpmsg PUBLIC -DEIGEN_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:eigentest>)
elseif (${APP} STREQUAL "UniPorton_test_libxml2_interface")
    add_subdirectory(${HOME_PATH}/testsuites/libxml2-test tmp)
    target_compile_options(rpmsg PUBLIC -DLIBXML2_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:libxml2> $<TARGET_OBJECTS:libxml2Test>)
elseif (${APP} STREQUAL "task-switch" OR
    ${APP} STREQUAL "task-preempt" OR
    ${APP} STREQUAL "semaphore-shuffle" OR
    ${APP} STREQUAL "interrupt-latency" OR
    ${APP} STREQUAL "deadlock-break" OR
    ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(rpmsg PUBLIC -DRHEALSTONE_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:rhealstoneTest>)
elseif (${APP} STREQUAL "libcclTest")
    add_subdirectory(${HOME_PATH}/src/component/ccl ccl)
    target_compile_options(rpmsg PUBLIC -DLIBCCL_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:ccl>)
elseif (${APP} STREQUAL "UniProton_soem_demo")
    if ("${CONFIG_OS_SUPPORT_SOEM}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/soemTest tmp)
        target_compile_options(rpmsg PUBLIC -DSOEM_DEMO)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:soemTest>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_SOEM")
        return()
    endif()
elseif(${APP} STREQUAL "UniPorton_test_log_interface")
    add_subdirectory(${HOME_PATH}/testsuites/log-test tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
    target_compile_options(config PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:logTest>)
elseif(${APP} STREQUAL "UniPorton_test_sem" OR
    ${APP} STREQUAL "UniPorton_test_rr_sched" OR
    ${APP} STREQUAL "UniPorton_test_mmu")
        add_subdirectory(${HOME_PATH}/testsuites/kern-test tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:kernTest>)
elseif (${APP} STREQUAL "netTest" OR
        ${APP} STREQUAL "uartTest")
    add_subdirectory(drivers)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:drivers>)
    target_compile_options(rpmsg PUBLIC -DDRIVER_TESTCASE)
else()
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
endif()

add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})

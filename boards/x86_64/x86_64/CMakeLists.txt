include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/x86_64
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/x86_64
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/ethercat
    ${HOME_PATH}/src/component/eigen-3.4.0
    ${HOME_PATH}/build/modbus/include/modbus
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/testsuites/modbusTest/
    ${HOME_PATH}/testsuites/forteTest/
    ${HOME_PATH}/src/arch/cpu/x86_64/common
    ${HOME_PATH}/src/arch/cpu/x86_64/common/hwi
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/build/libcxx/include
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/src/net/ethercat/include/
    ${HOME_PATH}/src/component/boost_1_54_0
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/drivers/i210
    ${HOME_PATH}/src/net/lwip_port
    ${HOME_PATH}/src/net/adapter/include
    ${HOME_PATH}/src/net/adapter/include/arch
    ${HOME_PATH}/src/net/lwip/src/include
    ${HOME_PATH}/src/osal/linux/include_self_def
    ${HOME_PATH}/src/include/uapi/
)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
if (${CONFIG_OS_SUPPORT_CXX})
add_subdirectory(${HOME_PATH}/src/component/boost_1_54_0 boost)
endif()

# 打印依赖代理接口
list(APPEND OBJS $<TARGET_OBJECTS:proxy>)

if (${APP} STREQUAL "UniPorton_test_posix_time_interface" OR 
    ${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_malloc_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_signal_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_exit_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(ethercat PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
endif()

if (${APP} STREQUAL "UniPorton_test_posix_math_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_stdlib_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_ipc_interface")
        add_subdirectory(${HOME_PATH}/testsuites/libc-test tmp)
        target_compile_options(ethercat PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest> $<TARGET_OBJECTS:common_mtest>)
endif()

if (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(ethercat PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
endif()

if (${APP} STREQUAL "UniPorton_test_libxml2_interface")
    add_subdirectory(${HOME_PATH}/testsuites/libxml2-test tmp)
    target_compile_options(ethercat PUBLIC -DLIBXML2_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:libxml2Test>)
endif()

if (${APP} STREQUAL "linuxTest")
add_subdirectory(${HOME_PATH}/testsuites/linux-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:ighTest>)
    target_compile_options(ethercat PUBLIC -DLINUX_TESTCASE)
endif()

if (${APP} STREQUAL "ethercatTest")
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
    target_compile_options(ethercat PUBLIC -DETHERCAT_TESTCASE)
endif()

if (${APP} STREQUAL "UniProton_modbus_demo")
    if ("${CONFIG_OS_OPTION_MODBUS}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/modbusTest tmp)
        target_compile_options(ethercat PUBLIC -DMODBUS_DEMO)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:modbusTest>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_OPTION_MODBUS")
        return()
    endif()
endif()

if (${APP} STREQUAL "UniProton_forte_demo")
    if ("${CONFIG_OS_OPTION_FORTE}" STREQUAL "y" AND "${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/forteTest tmp)
        target_compile_options(ethercat PUBLIC -DFORTE_DEMO)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:forteInit>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_OPTION_FORTE AND CONFIG_OS_SUPPORT_CXX")
        return()
    endif()
endif()

if (${APP} STREQUAL "UniProton_lwip_demo")
    if ("${CONFIG_OS_SUPPORT_NET}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/lwipTest tmp)
        target_compile_options(ethercat PUBLIC -DLWIP_DEMO)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:lwipUdpClient>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_NET")
        return()
    endif()
endif()

if (${APP} STREQUAL "libcclTest")
    add_subdirectory(${HOME_PATH}/src/component/ccl ccl)
    target_compile_options(ethercat PUBLIC -DLIBCCL_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:ccl>)
endif()

if (${APP} STREQUAL "cxxTest")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(../../testsuites/cxx-test tmp)
        target_compile_options(ethercat PUBLIC -DCXX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:cxxtest>)
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_CXX AND CONFIG_OS_OPTION_LOCALE")
        return()
    endif()
endif()

#rpmsg
if (${APP} MATCHES "x86_64")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost>)
    else()
        list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
    endif()
elseif(${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(ethercat PUBLIC -DRHEALSTONE_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:rhealstoneTest>)
endif()

add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})
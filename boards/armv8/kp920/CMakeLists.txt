include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/kp920
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/src/drivers/pcie
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/fs/include
    ${HOME_PATH}/src/component/proxy
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include
)

if (${CONFIG_OS_OPTION_RSC_TABLE})
    include_directories(${HOME_PATH}/src/component/mica)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp)
endif()

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
if (${CONFIG_OS_OPTION_PROXY})
    add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
    list(APPEND OBJS $<TARGET_OBJECTS:proxy>)
endif()

if (${CONFIG_OS_OPTION_RSC_TABLE})
    add_subdirectory(${HOME_PATH}/src/component/mica mica)
    list(APPEND OBJS $<TARGET_OBJECTS:mica>)
endif()

if (${APP} MATCHES "kp920")
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
elseif (${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(rpmsg PUBLIC -DRHEALSTONE_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:rhealstoneTest>)
elseif (${APP} STREQUAL "linuxTest")
    add_subdirectory(${HOME_PATH}/testsuites/linux-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:ighTest>)
    target_compile_options(rpmsg PUBLIC -DLINUX_TESTCASE)
elseif (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
endif()

add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})
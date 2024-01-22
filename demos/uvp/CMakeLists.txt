cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C CXX ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(OUTPUT_PATH ${HOME_PATH}/output)

set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_ASM_COMPILER ${CMAKE_ASM_COMPILER})

set(CC_OPTION "-g -O2 -mcmodel=large -static -nostdlib -nostdinc -fno-builtin -funwind-tables -nostartfiles -nodefaultlibs -mpreferred-stack-boundary=3 -mno-3dnow -mno-avx -mno-red-zone -Wl,--build-id=none -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -fno-stack-protector -funsigned-char -fno-PIC")
set(AS_OPTION "-c")
set(AR_OPTION "-r")
set(LD_OPTION "-static -Wl,-melf_x86_64 -Wl,--no-relax -Wl,-gc-sections -Wl,--eh-frame-hdr")
set(CMAKE_C_FLAGS "${CC_OPTION}")
set(CMAKE_CXX_FLAGS "${CC_OPTION}  -nostdinc++")
set(CMAKE_ASM_FLAGS "${AS_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/uvp_x86.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/uvp_x86.ld")
set(CMAKE_CXX_STANDARD 17)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_x86_uvp/defconfig)

set(LWIP_SRC_PATH ${HOME_PATH}/src/net/lwip)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/x86_64
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/lwip/output/include
    ${HOME_PATH}/src/net/lwip_port
    ${HOME_PATH}/src/arch/cpu/x86_64/common
    ${HOME_PATH}/src/arch/cpu/x86_64/common/hwi
    ${HOME_PATH}/platform/libboundscheck/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/include
    ${HOME_PATH}/src/drivers/i40e
    ${HOME_PATH}/src/osal/linux/include_self_def
    ${CMAKE_CURRENT_SOURCE_DIR}/component/lwip
    ${HOME_PATH}/src/net/adapter/include/arch
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    -Wl,--whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libuvp_x86.a"
    -Wl,--no-whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/liblwip.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/lib/libstdc++.a"
    "${TOOLCHAIN_PATH}/../lib64/gcc/x86_64-openeuler-linux-gnu/10.3.1/libgcc.a"
    "${TOOLCHAIN_PATH}/../lib64/gcc/x86_64-openeuler-linux-gnu/10.3.1/libgcc_eh.a"
    -Wl,--end-group
)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

# 打印依赖代理接口
list(APPEND OBJS $<TARGET_OBJECTS:proxy>)

if (${APP} STREQUAL "UniPorton_test_posix_time_interface" OR 
    ${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_malloc_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_signal_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_exit_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(openamp PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
        add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "UniPorton_test_posix_math_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_stdlib_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_ipc_interface")
        add_subdirectory(${HOME_PATH}/testsuites/libc-test tmp)
        target_compile_options(openamp PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest> $<TARGET_OBJECTS:common_mtest>)
        add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(openamp PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
    add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "UniPorton_test_libxml2_interface")
    add_subdirectory(${HOME_PATH}/testsuites/libxml2-test tmp)
    target_compile_options(openamp PUBLIC -DLIBXML2_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:libxml2Test>)
    add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "linuxTest")
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
    target_compile_options(openamp PUBLIC -DLINUX_TESTCASE)
    add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "libcclTest")
    add_subdirectory(${HOME_PATH}/src/component/ccl ccl)
    target_compile_options(openamp PUBLIC -DLIBCCL_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:ccl>)
    add_executable(${APP} ${OBJS})
endif()

if (${APP} STREQUAL "cxxTest")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(../../testsuites/cxx-test tmp)
        target_compile_options(openamp PUBLIC -DCXX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:cxxtest>)
        add_executable(${APP} ${OBJS})
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_CXX AND CONFIG_OS_OPTION_LOCALE")
        return()
    endif()
endif()

#rpmsg
if (${APP} MATCHES "x86_64")
        list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
        add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(openamp PUBLIC -DRHEALSTONE_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:rhealstoneTest>)
    add_executable(${APP} ${OBJS})
endif()

cmake_minimum_required(VERSION 3.12)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if (${APP} STREQUAL "cxxTest" OR ${APP} STREQUAL "eigenTest")
    set(CMAKE_C_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
    set(CMAKE_CXX_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-g++)
    set(CMAKE_ASM_COMPILER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
    set(CMAKE_LINKER ${CXX_TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-ld)
else()
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
    set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)
endif()

project(exmaples LANGUAGES C CXX ASM)

set(CC_OPTION "-g -march=armv8.2-a -nostdlib -nostdinc -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables")
set(AS_OPTION "-g -march=armv8.2-a -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy -Wl,-gc-sections -Wl,--eh-frame-hdr")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}  -nostdinc++")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/hi3093.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/hi3093.ld")
set(CMAKE_CXX_STANDARD 17)

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_hi3093/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/include/bits
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/hi3093
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/include
    ${HOME_PATH}/src/utility/lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/include
    ${HOME_PATH}/output/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/boost_1_54_0
    ${CMAKE_CURRENT_SOURCE_DIR}/component/eigen-3.4.0
    ${HOME_PATH}/output/soem/include
    ${HOME_PATH}/src/component/mica

    ${HOME_PATH}/output/libc/include
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/component/mica

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    -Wl,--whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libHI3093.a"
    -Wl,--no-whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/drivers/libs/libdriver.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
    -Wl,--end-group
)

if (${APP} STREQUAL "cxxTest" OR ${APP} STREQUAL "eigenTest")
    link_libraries(
        "${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/lib/libstdc++.a"
        "${CXX_TOOLCHAIN_PATH}/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/libgcc.a"
        "${CXX_TOOLCHAIN_PATH}/lib64/gcc/aarch64-openeuler-linux-gnu/10.3.1/libgcc_eh.a"
    )
endif()

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
if (${APP} STREQUAL "cxxTest" OR ${APP} STREQUAL "eigenTest")
add_subdirectory(component/boost_1_54_0)
endif()
if(${CONFIG_OS_SUPPORT_LIBXML2})
    add_subdirectory(component/libxml2)
endif()

if (${CONFIG_OS_OPTION_OPENAMP})
    add_subdirectory(${HOME_PATH}/src/component/mica mica)
    list(APPEND OBJS $<TARGET_OBJECTS:mica>)
endif()

if(${CONFIG_OS_SUPPORT_OPC_UA})
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/component/open62541-1.4.4/include
        ${CMAKE_CURRENT_SOURCE_DIR}/component/open62541-1.4.4/plugins/include
        ${CMAKE_CURRENT_SOURCE_DIR}/component/open62541-1.4.4/deps
        ${CMAKE_CURRENT_SOURCE_DIR}/component/open62541-1.4.4/src
        ${CMAKE_CURRENT_SOURCE_DIR}/component/open62541-1.4.4/src_generated
    )
    add_subdirectory(component/open62541-1.4.4)
    list(APPEND OBJS $<TARGET_OBJECTS:open62541>)
    link_libraries("${TOOLCHAIN_PATH}/lib/gcc/aarch64-none-elf/10.3.1/libgcc.a")
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
        add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
    add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "cxxTest")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/cxx-test tmp)
        target_compile_options(rpmsg PUBLIC -DCXX_TESTCASE)
        target_compile_options(cxxtest PUBLIC -D_POSIX_THREADS -DBOOST_HAS_PTHREAD_MUTEXATTR_SETTYPE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:cxxtest>)
        add_executable(${APP} ${OBJS})
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_CXX AND CONFIG_OS_OPTION_LOCALE")
        return()
    endif()
elseif(${APP} STREQUAL "eigenTest")
    add_subdirectory(${HOME_PATH}/testsuites/eigen-test tmp)
    target_compile_options(rpmsg PUBLIC -DEIGEN_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:boost> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:eigentest>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniPorton_test_libxml2_interface")
    add_subdirectory(${HOME_PATH}/testsuites/libxml2-test tmp)
    target_compile_options(rpmsg PUBLIC -DLIBXML2_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:libxml2> $<TARGET_OBJECTS:libxml2Test>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "task-switch" OR
    ${APP} STREQUAL "task-preempt" OR
    ${APP} STREQUAL "semaphore-shuffle" OR
    ${APP} STREQUAL "interrupt-latency" OR
    ${APP} STREQUAL "deadlock-break" OR
    ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(rpmsg PUBLIC -DRHEALSTONE_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:rhealstoneTest>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "libcclTest")
    add_subdirectory(${HOME_PATH}/src/component/ccl ccl)
    target_compile_options(rpmsg PUBLIC -DLIBCCL_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:ccl>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniProton_soem_demo")
    if ("${CONFIG_OS_SUPPORT_SOEM}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/soemTest tmp)
        target_compile_options(rpmsg PUBLIC -DSOEM_DEMO)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:soemTest>)
        add_executable(${APP} ${OBJS})
    else()
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_SOEM")
        return()
    endif()
elseif(${APP} STREQUAL "UniPorton_test_log_interface")
    add_subdirectory(${HOME_PATH}/testsuites/log-test tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
    target_compile_options(config PUBLIC -DPOSIX_TESTCASE -DLOG_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:logTest>)
    add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "UniPorton_test_sem" OR
    ${APP} STREQUAL "UniPorton_test_rr_sched" OR
    ${APP} STREQUAL "UniPorton_test_mmu")
        add_subdirectory(${HOME_PATH}/testsuites/kern-test tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:kernTest>)
        add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "netTest" OR
        ${APP} STREQUAL "uartTest")
    add_subdirectory(drivers)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:drivers>)
    target_compile_options(rpmsg PUBLIC -DDRIVER_TESTCASE)
    add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "opcuaTest")
    add_subdirectory(${HOME_PATH}/testsuites/opcuaTest tmp)
    target_compile_options(rpmsg PUBLIC -DOPCUA_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:opcuaServer>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniProton_jitter_test")
    add_subdirectory(${HOME_PATH}/testsuites/jitter-test tmp)
    target_compile_options(rpmsg PUBLIC -DJITTER_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:jitterTest>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniProton_cyclictest")
    add_subdirectory(${HOME_PATH}/testsuites/cyclictest tmp)
    target_compile_options(rpmsg PUBLIC -DCYCLIC_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:cyclicTest>)
    add_executable(${APP} ${OBJS})
else()
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
    if (${CONFIG_LOSCFG_SHELL_TEST})
        add_subdirectory(${HOME_PATH}/testsuites/cyclictest tmp)
        add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp1)
        list(APPEND OBJS $<TARGET_OBJECTS:cyclicTest> $<TARGET_OBJECTS:rhealstoneTest>)
    endif()
    add_executable(${APP} ${OBJS})
endif()

if(${CONFIG_LOSCFG_SHELL})
set(LD_OPTS)
list(APPEND LD_OPTS
    -uhelp_shellcmd
    -umemInfo_shellcmd
    -utaskInfo_shellcmd
    -uuname_shellcmd
    -uhwi_shellcmd
    -ucpup_shellcmd
    -usem_shellcmd
    -uswtmr_shellcmd
    -uqueue_shellcmd
    -usysteminfo_shellcmd
)
target_link_options(${APP} PUBLIC ${LD_OPTS})
endif()

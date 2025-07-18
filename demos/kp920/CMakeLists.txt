cmake_minimum_required(VERSION 3.12)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(OUTPUT_PATH ${HOME_PATH}/output)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)

project(exmaples LANGUAGES C ASM)

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdinc -nostdlib")
set(AS_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy -Wl,--gc-sections -Wl,--undefined=taskInfo_shellcmd -Wl,--undefined=help_shellcmd -Wl,--undefined=memInfo_shellcmd")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_kp920/defconfig)

if (${CONFIG_OS_OPTION_RSC_TABLE})
    set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/kp920_rsc.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/kp920_rsc.ld")
else()
    set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/kp920.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/kp920.ld")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/kp920
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/src/drivers/pcie
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/fs/include
    ${HOME_PATH}/src/component/proxy
    ${OUTPUT_PATH}/libc/include
    ${OUTPUT_PATH}/libc/include/bits
)

if (${CONFIG_OS_OPTION_RSC_TABLE})
    include_directories(${HOME_PATH}/src/component/mica)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp)
endif()

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libKP920.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
    -Wl,--end-group
)

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
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "rcv-data" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    if (${APP} STREQUAL "rcv-data")
        target_compile_options(rpmsg PUBLIC -DRCV_TEST)
    else()
        target_compile_options(rpmsg PUBLIC -DRHEALSTONE_TESTCASE)
    endif()
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:rhealstoneTest>)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "linuxTest")
    add_subdirectory(${HOME_PATH}/testsuites/linux-test tmp)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:ighTest>)
    target_compile_options(rpmsg PUBLIC -DLINUX_TESTCASE)
    add_executable(${APP} ${OBJS})
elseif (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
    add_executable(${APP} ${OBJS})
endif()

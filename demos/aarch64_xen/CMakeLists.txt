cmake_minimum_required(VERSION 3.12)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(OUTPUT_PATH ${HOME_PATH}/output)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)

project(exmaples LANGUAGES C ASM)
add_compile_definitions(_GNU_SOURCE)

# build_app.sh prepares OS archives, headers and ordinary static installs.
# -nostdinc still needs this selected compiler's freestanding C11 headers.
execute_process(COMMAND "${CMAKE_C_COMPILER}" -print-file-name=include
    OUTPUT_VARIABLE COMPILER_BUILTIN_INCLUDE OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE COMPILER_INCLUDE_RESULT)
if(NOT COMPILER_INCLUDE_RESULT EQUAL 0 OR NOT IS_DIRECTORY "${COMPILER_BUILTIN_INCLUDE}")
    message(FATAL_ERROR "Selected compiler does not expose its builtin include directory")
endif()

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -march=armv8.2-a -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdinc -nostdlib")
set(AS_OPTION "-g -march=armv8.2-a -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION} -isystem ${COMPILER_BUILTIN_INCLUDE}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/aarch64_xen.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/aarch64_xen.ld")

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_aarch64_xen/defconfig)
if(NOT "${CONFIG_OS_OPTION_LIBMICA}" STREQUAL "y")
    message(FATAL_ERROR "This libmica demo requires CONFIG_OS_OPTION_LIBMICA=y")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/aarch64_xen
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/include
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/src/drivers/pcie
    ${HOME_PATH}/src/shell/adapter/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/fs/include
    ${HOME_PATH}/src/component/proxy
    ${UNIPROTON_INCLUDE_DIRS}
)

if(CONFIG_OS_OPTION_LIBMICA)
    include_directories(
        ${MICA_INSTALL_PREFIX}/include
        ${MICA_LIBMETAL_ROOT}/include
        ${MICA_OPENAMP_ROOT}/include
        ${HOME_PATH}/src/component/libmica_adapter/include
    )
endif()

if (${CONFIG_OS_OPTION_PROXY})
    add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
    list(APPEND OBJS $<TARGET_OBJECTS:proxy>)
endif()

# Old four-source MICA OBJECT library and old Xen facilities are not linked.

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
if(CONFIG_OS_OPTION_LIBMICA)
    add_subdirectory(${HOME_PATH}/src/component/libmica_adapter libmica_adapter)
    list(APPEND OBJS $<TARGET_OBJECTS:uniproton_mica_adapter>)
endif()

if (${APP} STREQUAL "UniPorton_test_posix_time_interface" OR 
    ${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface" OR
    ${APP} STREQUAL "UniPorton_test_posix_signal_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(openamp PUBLIC -DTESTSUITE_CASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
    add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "task-switch" OR
        ${APP} STREQUAL "task-preempt" OR
        ${APP} STREQUAL "semaphore-shuffle" OR
        ${APP} STREQUAL "interrupt-latency" OR
        ${APP} STREQUAL "deadlock-break" OR
        ${APP} STREQUAL "message-latency")
    add_subdirectory(${HOME_PATH}/testsuites/rhealstone tmp)
    target_compile_options(openamp PUBLIC -DTESTSUITE_CASE)
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:rhealstoneTest>)
    add_executable(${APP} ${OBJS})
else()
    list(APPEND OBJS $<TARGET_OBJECTS:openamp> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
    add_executable(${APP} ${OBJS})
endif()

# MICA/SDK use OS libc/pthread, and OS libc's vprintf calls the demo TTY sink.
# Rescan this explicit dependency cycle, not whole-archive any owner's objects.
if(CONFIG_OS_OPTION_LIBMICA)
    target_link_libraries(${APP} PRIVATE -Wl,--start-group
        "${MICA_INSTALL_PREFIX}/lib/libmica.a"
        "${MICA_OPENAMP_ROOT}/lib/libopen_amp.a"
        "${MICA_LIBMETAL_ROOT}/lib/libmetal.a"
        "${MICA_UNIPROTON_ARCHIVE}"
        "${MICA_BOUNDSCHECK_ARCHIVE}"
        gcc -Wl,--end-group)
    target_link_options(${APP} PRIVATE "LINKER:--undefined=rsc_table_get")
endif()
target_link_options(${APP} PRIVATE "LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${APP}.map")

if(${CONFIG_LOSCFG_SHELL})
set(LD_OPTS)
list(APPEND LD_OPTS
    -uhelp_shellcmd
    -umemInfo_shellcmd
    -utaskInfo_shellcmd
    -uuname_shellcmd
    -uhwi_shellcmd
    -usem_shellcmd
    -uswtmr_shellcmd
    -uqueue_shellcmd
    -usysteminfo_shellcmd
)
if(CONFIG_OS_OPTION_CPUP)
    list(APPEND LD_OPTS -ucpup_shellcmd)
endif()
target_link_options(${APP} PUBLIC ${LD_OPTS})
endif()

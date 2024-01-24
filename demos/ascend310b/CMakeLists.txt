cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(OUTPUT_PATH ${HOME_PATH}/output)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)

set(POSIX_OPTION "-D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
set(CC_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables -nostdlib -nostdinc")
set(AS_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")

# 公共函数定义导入
include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
# 函数import_kconfig把.config文件中的定义转换为cmake变量，用于后续子文件夹中决定是否编译某库
import_kconfig(${HOME_PATH}/build/uniproton_config/config_armv8_ascend310b/defconfig)

if (${CONFIG_OS_OPTION_OPENAMP})
    set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/ascend310b_openamp.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/ascend310b_openamp.ld")
    set(CC_OPTION  "-O0")
    set(AS_OPTION  "-O0")
elseif(${CONFIG_GUEST_OS})
    set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/ascend310b_guestos.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/ascend310b_guestos.ld")
    set(CC_OPTION  "-O2")
    set(AS_OPTION  "-O2")
endif()

if (${CONFIG_OS_OPTION_OPENAMP})
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp/ascend310b
        ${HOME_PATH}/src/arch/include
        ${HOME_PATH}/src/utility/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
        ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
        ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
        ${HOME_PATH}/output/libc/include
        ${HOME_PATH}/src/shell/adapter/include
        ${HOME_PATH}/src/shell/full/include
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

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)

if (${CONFIG_OS_OPTION_OPENAMP})
    link_libraries(
        -Wl,--start-group
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libASCEND310B.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
        -Wl,--end-group
    )
elseif(${CONFIG_GUEST_OS})
    link_libraries(
        -Wl,--start-group
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/libASCEND310B.a"
        -Wl,--end-group
    )
endif()

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
list(APPEND OBJS $<TARGET_OBJECTS:proxy>)

if (${CONFIG_OS_OPTION_OPENAMP})
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
elseif(${CONFIG_GUEST_OS})
    list(APPEND OBJS $<TARGET_OBJECTS:apps> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
endif()
add_executable(${APP} ${OBJS})

if(${CONFIG_LOSCFG_SHELL})
set(LD_OPTS)
list(APPEND LD_OPTS
    -uhelp_shellcmd
    -umemInfo_shellcmd
    -utaskInfo_shellcmd
)
target_link_options(${APP} PUBLIC ${LD_OPTS})
endif()

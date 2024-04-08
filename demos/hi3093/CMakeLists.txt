cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C CXX ASM)

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

set(CC_OPTION "-g -march=armv8.2-a+nosimd -nostdlib -nostdinc -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables")
set(AS_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/libcxx/include
    ${HOME_PATH}/output/libc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/component/boost_1_54_0
    ${CMAKE_CURRENT_SOURCE_DIR}/component/eigen-3.4.0
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    -Wl,--whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libHI3093.a"
    -Wl,--no-whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
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

if (${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
    target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:posixTest>)
    add_executable(${APP} ${OBJS})
elseif(${APP} STREQUAL "cxxTest")
    if ("${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y" AND "${CONFIG_OS_OPTION_LOCALE}" STREQUAL "y")
        add_subdirectory(${HOME_PATH}/testsuites/cxx-test tmp)
        target_compile_options(rpmsg PUBLIC -DCXX_TESTCASE)
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
elseif (${APP} STREQUAL "libcclTest")
    add_subdirectory(${HOME_PATH}/src/component/ccl ccl)
    target_compile_options(rpmsg PUBLIC -DLIBCCL_TESTCASE)
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart> $<TARGET_OBJECTS:ccl>)
    add_executable(${APP} ${OBJS})
else()
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
    add_executable(${APP} ${OBJS})
endif()

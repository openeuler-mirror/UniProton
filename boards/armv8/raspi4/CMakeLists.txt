cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-openeuler-linux-ld)

set(CC_OPTION "-g -march=armv8.2-a+nofp+nosimd -nostdlib -nostdinc -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables")
set(AS_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/raspi4.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/raspi4.ld")

include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/raspi4
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${CMAKE_CURRENT_SOURCE_DIR}/build/open-amp/output/usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/output/libc/include
    ${HOME_PATH}/src/component/mica
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/build/output/libc/include
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    -Wl,--start-group
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libCortexMXsec_c.lib"
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libRASPI4.a"
    "${HOME_PATH}/build/libmetal/output/usr/local/lib/libmetal.a"
    "${HOME_PATH}/build/open-amp/output/usr/local/lib/libopen_amp.a"
    -Wl,--end-group
)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(${HOME_PATH}/src/component/mica mica)
list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:mica>)
add_executable(${APP} ${OBJS})

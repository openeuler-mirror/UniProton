cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(TOOLCHAIN_PREFIX "riscv64-unknown-elf-")

if(DEFINED ENV{RISCV_NATIVE} AND "$ENV{RISCV_NATIVE}" STREQUAL "true")
    set(TOOLCHAIN_PREFIX "")
endif()

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}ld)
set(CC_OPTION "-march=rv64gczbb -mabi=lp64d -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal -Wshadow -Wtrampolines -Wdate-time  -static -nostdlib -nostartfiles -nodefaultlibs  -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm   -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns  -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -nostartfiles -fno-stack-protector  -funsigned-char -fno-PIC -std=c99 -ffreestanding -nostdinc -Wno-undef -Wno-strict-prototypes -Wno-redundant-decls -Wno-cast-qual -Wno-parentheses -Wno-pointer-sign -Wno-unused-parameter -Wno-sign-compare -Wno-jump-misses-init -Wno-old-style-definition -Wno-shadow -Wno-unused-macros -mcmodel=medany -static" )
set(AS_OPTION "-march=rv64gczbb -mabi=lp64d -mcmodel=medany -static -fno-builtin -fno-PIE -Wall -fomit-frame-pointer  -fzero-initialized-in-bss -fno-common ")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy")
set(CMAKE_C_FLAGS "${CC_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION}")

set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/shell.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/shell.ld")

include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_riscv64_d1s/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hw/riscv64
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/driver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/misc_m
    ${HOME_PATH}/output/libc/include
    ${HOME_PATH}/output/shell/adapter/include
    ${HOME_PATH}/output/shell/include
    ${HOME_PATH}/libboundscheck/include
)

if(DEFINED ENV{RISCV_NATIVE} AND "$ENV{RISCV_NATIVE}" STREQUAL "true")
  execute_process(
    COMMAND gcc -print-libgcc-file-name
    OUTPUT_VARIABLE GCC_LIBGCC_FILE_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  string(LENGTH "/usr/lib/gcc/" PREFIX_LENGTH)
  string(SUBSTRING "${GCC_LIBGCC_FILE_NAME}" ${PREFIX_LENGTH} -1 LIB_PATH)

  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${TOOLCHIAN_PATH}/lib/gcc/${LIB_PATH}
  )
  link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libDS-D1S.a"
    "${TOOLCHAIN_PATH}/lib/gcc/${LIB_PATH}"
    -Wl,--end-group
  )
else()
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs
		 ${TOOLCHIAN_PATH}/lib/gcc/riscv64-unknown-elf/13.2.0
  )
  link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libDS-D1S.a"
    "${TOOLCHAIN_PATH}/lib/gcc/riscv64-unknown-elf/13.2.0/libgcc.a"
    -Wl,--end-group
  )
endif()

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

list(APPEND OBJS $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:app>)
add_executable(${APP} ${OBJS})

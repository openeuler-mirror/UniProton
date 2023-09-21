cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)

set(CC_OPTION "-g -march=armv8.2-a+nofp+nosimd -nostdinc -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -specs=nosys.specs -nostartfiles -funwind-tables")
set(AS_OPTION "-g -march=armv8.2-a+nofp+nosimd -Wl,--build-id=none -fno-builtin -fno-PIE -Wall -fno-dwarf2-cfi-asm -O0 -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -mgeneral-regs-only -DENV_EL1")
set(LD_OPTION "-static -no-pie -Wl,--wrap=memset -Wl,--wrap=memcpy")
set(CMAKE_C_FLAGS "${CC_OPTION} ${POSIX_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ${POSIX_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ../raspi4.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ../raspi4.ld")

#set(CONFIG_OS_GDB_STUB y)

if (${CONFIG_OS_GDB_STUB})
add_compile_options(-DOS_GDB_STUB)
endif()

include_directories(
    ./include
    ./include/libc/include
    ./config
    ./bsp
    ./bsp/raspi4
    ./apps/openamp
    ./build/open-amp/output/usr/local/include
    ./build/libmetal/output/usr/local/include
    ../../output/libc/include
)

link_directories(./libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libRASPI4.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
    -Wl,--end-group
)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

if (RPC_DEMO_SWITCH)
add_subdirectory(../../src/component/proxy proxy)
list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
else()
list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
endif()
add_executable(${APP} ${OBJS})

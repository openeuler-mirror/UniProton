cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C CXX ASM)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/x86_64-openeuler-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/x86_64-openeuler-linux-gnu-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/x86_64-openeuler-linux-gnu-as)

set(CC_OPTION "-g -mcmodel=large -static -nostdlib -nostdinc -fno-builtin -funwind-tables -nostartfiles -nodefaultlibs -mpreferred-stack-boundary=3 -mno-3dnow -mno-avx -mno-red-zone -Wl,--build-id=none -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -fno-stack-protector -funsigned-char -fno-PIC -nostdinc++")
set(AS_OPTION "-c")
set(AR_OPTION "-r")
set(LD_OPTION "-static -Wl,-melf_x86_64 -Wl,--no-relax -Wl,-gc-sections -Wl,--wrap=memset -Wl,--wrap=memcpy -Wl,--wrap=strlen -Wl,--wrap=strnlen -Wl,--wrap=memchr -Wl,--wrap=strncpy  -Wl,--wrap=memcmp -Wl,--wrap=strcmp -Wl,--wrap=strncmp")
set(CMAKE_C_FLAGS "${CC_OPTION}")
set(CMAKE_CXX_FLAGS "${CC_OPTION}")
set(CMAKE_ASM_FLAGS "${AS_OPTION}")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/uvpck.ld")
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/build/uvpck.ld")
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ./include
    ./include/libc/include
    ./include/libc/include/bits
    ./config
    ./bsp
    ./bsp/uvpck
    ./component/ethercat-master/include
    ./component/ethercat-master/master
    ./apps/ethercat
    ./build/open-amp/output/usr/local/include
    ./build/libmetal/output/usr/local/include
    ../../src/arch/cpu/x86_64/common
    ../../src/arch/cpu/x86_64/common/hwi
    ../../platform/libboundscheck/include
    /usr1/openeuler/gcc/openeuler_gcc_x86_64/x86_64-openeuler-linux-gnu/include/c++/10.3.1/
    /usr1/openeuler/gcc/openeuler_gcc_x86_64/x86_64-openeuler-linux-gnu/include/c++/10.3.1/x86_64-openeuler-linux-gnu
)

link_directories(./libs)
link_libraries(
    -Wl,--start-group
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib"
    -Wl,--whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libUVPCK.a"
    -Wl,--no-whole-archive
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libmetal.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libopen_amp.a"
    "/usr1/openeuler/gcc/openeuler_gcc_x86_64/x86_64-openeuler-linux-gnu/lib64/libstdc++.a"
    "/usr1/openeuler/gcc/openeuler_gcc_x86_64/lib64/gcc/x86_64-openeuler-linux-gnu/10.3.1/libgcc.a"
    "/usr1/openeuler/gcc/openeuler_gcc_x86_64/lib64/gcc/x86_64-openeuler-linux-gnu/10.3.1/libgcc_eh.a"
    -Wl,--end-group
)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(component/ethercat-master)

#rpmsg
if (${APP} MATCHES "uvpck")
    list(APPEND OBJS $<TARGET_OBJECTS:ethercat> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:ethercat_src>)
    add_executable(${APP} ${OBJS})
endif()


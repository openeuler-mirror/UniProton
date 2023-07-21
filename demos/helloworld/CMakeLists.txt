cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM CXX)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(CMAKE_C_FLAGS "-g -O2 -mthumb -mcpu=cortex-m4 -std=gnu11 -fno-common -fomit-frame-pointer -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb  -fstack-protector-strong  -funsigned-char -fdata-sections -ffunction-sections -fshort-enums")
set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m4 -fno-threadsafe-statics -fno-builtin -DEIGEN_NO_IO=1")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS})

set(CXX_LIB ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crti.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtbegin.o
            ${TOOLCHAIN_PATH}/arm-none-eabi/lib/crt0.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtend.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtn.o)

if("${SIM}" STREQUAL "_SIM_")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${SIM}")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ../qemu_m4.ld")
else()
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ../examples.ld")
endif()

include_directories(./include ./config ./bsp)
link_directories(./libs)
link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexM4.a" "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib" "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexM4.a" "${CMAKE_CURRENT_SOURCE_DIR}/libs/libCortexMXsec_c.lib")
link_libraries(-lm -lc_nano -lnosys -lgcc)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

list(APPEND OBJS $<TARGET_OBJECTS:apps> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
add_executable(${APP} ${CXX_LIB} ${OBJS})

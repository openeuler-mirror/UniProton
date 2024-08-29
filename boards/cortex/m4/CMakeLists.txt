cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM CXX)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(CMAKE_C_FLAGS "-g -O0 -mthumb -mcpu=cortex-m4 -std=gnu11 -fno-common -fomit-frame-pointer -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb  -fstack-protector-strong  -funsigned-char -fdata-sections -ffunction-sections -fshort-enums")
set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m4 -fno-threadsafe-statics -fno-builtin -DEIGEN_NO_IO=1")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS})

include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/defconfig)

set(CXX_LIB ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crti.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtbegin.o
            ${TOOLCHAIN_PATH}/arm-none-eabi/lib/crt0.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtend.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtn.o)

if(${APP} STREQUAL "helloworld")
    if("${SIM}" STREQUAL "_SIM_")
        set(ldfile "${CMAKE_CURRENT_SOURCE_DIR}/build/qemu_helloworld.ld")
    else()
        set(ldfile "${CMAKE_CURRENT_SOURCE_DIR}/build/helloworld.ld")
    endif()
elseif(${APP} STREQUAL "shell")
    set(ldfile "${CMAKE_CURRENT_SOURCE_DIR}/build/shell.ld")
elseif(${APP} STREQUAL "stdio" OR ${APP} STREQUAL "fatfs")
    set(ldfile "${CMAKE_CURRENT_SOURCE_DIR}/build/fatfs.ld")
endif()

if("${SIM}" STREQUAL "_SIM_")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${SIM}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ${ldfile}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${HOME_PATH}/build/output/libc/include
    ${HOME_PATH}/build/output/UniProton/include
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/
)

if(${APP} STREQUAL "shell" OR ${APP} STREQUAL "fatfs")
include_directories(
                    ${HOME_PATH}/build/output/drivers/include/
                    ${HOME_PATH}/build/output/shell/include/
                    ${HOME_PATH}/build/output/shell/adapter/include
                    )
endif()
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
link_libraries(
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libCortexM4.a" 
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libCortexMXsec_c.lib"
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libCortexM4.a"
    "${HOME_PATH}/build/output/${CPU_TYPE}/${ARCH_TYPE}/FPGA/libCortexMXsec_c.lib")
link_libraries(-lgcc)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

list(APPEND OBJS $<TARGET_OBJECTS:apps> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
add_executable(${APP} ${CXX_LIB} ${OBJS})
if(${APP} STREQUAL "shell")
set(LD_OPTS )
list(APPEND LD_OPTS
    -uhelp_shellcmd
    -ulog_shellcmd
)
target_link_options(${APP} PUBLIC ${LD_OPTS})
endif()

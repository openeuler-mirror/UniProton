cmake_minimum_required(VERSION 3.12)
project(exmaples LANGUAGES C ASM CXX)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++)
set(CMAKE_C_FLAGS "-g -O2 -mthumb -mcpu=cortex-m4 -std=gnu11 -fno-common -fomit-frame-pointer -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb  -fstack-protector-strong  -funsigned-char -fdata-sections -ffunction-sections -fshort-enums")
set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m4 -fno-threadsafe-statics -fno-builtin -DEIGEN_NO_IO=1")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_ASM_FLAGS ${CMAKE_C_FLAGS})
set(UNIPROTON_PROJECT_DIR ${HOME_PATH})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/${APP})

set(CXX_LIB ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crti.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtbegin.o
            ${TOOLCHAIN_PATH}/arm-none-eabi/lib/crt0.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtend.o
            ${TOOLCHAIN_PATH}/lib/gcc/arm-none-eabi/10.2.1/crtn.o)

if("${SIM}" STREQUAL "_SIM_")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${SIM}")
    set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ${CMAKE_CURRENT_SOURCE_DIR}/build/qemu_m4.ld")
else()
    set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-EL -Wl,-d -Wl,-no-enum-size-warning  -u _printf_float -nostartfiles -static -T ${CMAKE_CURRENT_SOURCE_DIR}/build/examples.ld")
endif()

# 公共函数定义导入
include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)
# 函数import_kconfig把.config文件中的定义转换为cmake变量，用于后续子文件夹中决定是否编译某库
import_kconfig(${HOME_PATH}/build/uniproton_config/config_m4/defconfig)

set(OUTPUT_PATH ${HOME_PATH}/output)
include_directories(
                    ${OUTPUT_PATH}/libc/include
                    ${OUTPUT_PATH}/UniProton/include
                    ${OUTPUT_PATH}/libboundscheck/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/config
                    ${HOME_PATH}/demos/m4/bsp
                    ${HOME_PATH}/build/uniproton_config/config_m4
                    ${HOME_PATH}/src/include/uapi)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(M4_UNIPROTON_LIB ${OUTPUT_PATH}/UniProton/lib/cortex_m4/libCortexM4.a)
set(M4_PLATFORM_LIB ${OUTPUT_PATH}/libboundscheck/lib/cortex_m4/libCortexMXsec_c.lib)
link_libraries(${M4_UNIPROTON_LIB} ${M4_PLATFORM_LIB} ${M4_UNIPROTON_LIB} ${M4_PLATFORM_LIB})
link_libraries(-lgcc)

add_subdirectory(${HOME_PATH}/demos/m4/bsp bsp)
add_subdirectory(config)
add_subdirectory(support)

add_subdirectory(posixtestsuite)
add_subdirectory(rhealstone)
add_subdirectory(drivers)
add_subdirectory(shell-test)
add_subdirectory(libc-test)
add_subdirectory(libxml2-test)

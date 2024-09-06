include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

set(CXX_LIB 
    ${TOOLCHAIN_DIR}/../lib/gcc/arm-none-eabi/10.2.1/crti.o
    ${TOOLCHAIN_DIR}/../lib/gcc/arm-none-eabi/10.2.1/crtbegin.o
    ${TOOLCHAIN_DIR}/../arm-none-eabi/lib/crt0.o
    ${TOOLCHAIN_DIR}/../lib/gcc/arm-none-eabi/10.2.1/crtend.o
    ${TOOLCHAIN_DIR}/../lib/gcc/arm-none-eabi/10.2.1/crtn.o)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
)

if(${APP} STREQUAL "shell" OR ${APP} STREQUAL "fatfs")
include_directories(
    ${HOME_PATH}/src/arch/arm/include
    ${HOME_PATH}/src/fs/include
    ${HOME_PATH}/src/shell/full/include
    ${HOME_PATH}/src/shell/adapter/include
)
endif()

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

list(APPEND OBJS $<TARGET_OBJECTS:apps> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config>)
add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS} ${CXX_LIB})

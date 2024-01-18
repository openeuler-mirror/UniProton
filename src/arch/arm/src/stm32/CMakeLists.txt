file(GLOB_RECURSE STM32_ARCH_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
list(REMOVE_ITEM STM32_ARCH_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f40xxx_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f40xxx_rtcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f20xxf40xx_flash.c
)

if(NOT DEFINED CONFIG_OS_OPTION_STM32_FLASH OR NOT ${CONFIG_OS_OPTION_STM32_FLASH})
list(REMOVE_ITEM STM32_ARCH_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32_flash.c
)
endif()

add_custom_target(copy_board_h ALL)
add_custom_command(TARGET copy_board_h
                PRE_BUILD
                COMMAND mkdir -p ${SRC_ARCH_DIRECTORY}/arm/include/arch/board/
                COMMAND cp ${DRIVERS_BASE_DIR}/board/stm32/stm32f407/board.h ${SRC_ARCH_DIRECTORY}/arm/include/arch/board/
)
add_library(stm32_arch OBJECT ${STM32_ARCH_SOURCE})
add_dependencies(stm32_arch copy_board_h)
target_include_directories(stm32_arch PUBLIC 
    ${SRC_ARCH_DIRECTORY}/arm/include
    ${SRC_ARCH_DIRECTORY}/arm/include/common
    ${SRC_ARCH_DIRECTORY}/arm/include/stm32
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)
if ("${CONFIG_OS_OPTION_STM32_FLASH}")
target_include_directories(stm32_arch PUBLIC 
    ${FS_BASE_DIR}/fat/ff15/source
)
endif()

target_compile_options(stm32_arch PUBLIC 
        -Wno-shift-overflow
        -Wno-unused-function
        )

list(APPEND ALL_OBJECT_LIBRARYS stm32_arch)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
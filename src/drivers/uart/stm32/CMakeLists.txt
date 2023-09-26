file(GLOB_RECURSE STM32_UART_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
list(REMOVE_ITEM STM32_UART_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f40xxx_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f40xxx_rtcc.c
)
add_custom_target(copy_board_h ALL)
add_custom_command(TARGET copy_board_h
                PRE_BUILD
                COMMAND mkdir -p ${DRIVERS_BASE_DIR}/include/arch/board/
                COMMAND cp ${DRIVERS_BASE_DIR}/board/stm32/stm32f407/board.h ${DRIVERS_BASE_DIR}/include/arch/board/
)
add_library(stm32_uart OBJECT ${STM32_UART_SOURCE})
add_dependencies(stm32_uart copy_board_h)
target_include_directories(stm32_uart PUBLIC 
    ${DRIVERS_BASE_DIR}/include
    ${DRIVERS_BASE_DIR}/include/arch/arm/common
    ${DRIVERS_BASE_DIR}/include/arch/arm/stm32
    ${DRIVERS_BASE_DIR}/base
)

target_compile_options(stm32_uart PUBLIC 
        -Wno-shift-overflow
        -Wno-unused-function
        )

list(APPEND ALL_OBJECT_LIBRARYS stm32_uart)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
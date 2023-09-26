if(NOT "${CONFIG_OS_OPTION_NUTTX_VFS}")
    RETURN()
endif()

set(DRIVERS_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(adapter)
add_subdirectory(base)

if (${CONFIG_OS_OPTION_NUTTX_UART})
    add_subdirectory(uart)
    if(${CONFIG_OS_OPTION_STM32F407_UART})
        add_subdirectory(uart/stm32)
    endif()
endif()

install(DIRECTORY
    ${DRIVERS_BASE_DIR}/include
    DESTINATION drivers/
)
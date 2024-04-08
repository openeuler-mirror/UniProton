add_subdirectory(i210)
if (${CONFIG_OS_OPTION_PCIE})
    add_subdirectory(pcie)
endif()

if (${CONFIG_OS_SUPPORT_I40E})
    add_subdirectory(i40e)
endif()

if(NOT "${CONFIG_OS_OPTION_NUTTX_VFS}")
    RETURN()
endif()

if(NOT "${CONFIG_OS_OPTION_DRIVER}")
    RETURN()
endif()

set(DRIVERS_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FS_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../fs/)
set(NET_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../net/)

if (${CONFIG_OS_OPTION_NUTTX_UART})
    add_subdirectory(uart)
endif()

if (${CONFIG_CONFIG_PIPES})
    add_subdirectory(pipes)
endif()

if (${CONFIG_CONFIG_MTD_PROGMEM})
    add_subdirectory(mtd)
endif()

if ("${CONFIG_CONFIG_NET}")
    add_subdirectory(net)
endif()

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/src/arch/arm/include
    DESTINATION drivers/
)
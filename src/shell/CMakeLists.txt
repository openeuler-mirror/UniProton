
message("======== Compile Shell dir  ========")

file(GLOB_RECURSE SHELL_SOURCE full/*.c adapter/src/los_*.c)

if(${CONFIG_LOSCFG_SHELL_MICA_INPUT})
    list(APPEND SHELL_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/src/uart_adap_mica.c
    )
elseif(${CONFIG_OS_OPTION_NUTTX_UART})
    list(APPEND SHELL_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/src/uart_adap_nuttx.c
    )
else()
    list(APPEND SHELL_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/src/uart_adap_empty.c
    )
endif()

if(NOT "${CONFIG_OS_OPTION_NUTTX_VFS}")
    list(REMOVE_ITEM SHELL_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/full/src/base/porting/port_uart.c
    )
endif()

add_library(libshell OBJECT ${SHELL_SOURCE})

target_include_directories(libshell PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/full/include
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include
)

list(APPEND ALL_OBJECT_LIBRARYS libshell)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/full/include/shell.h
    ${CMAKE_CURRENT_SOURCE_DIR}/full/include/show.h
    DESTINATION shell/include/
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_printf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/console.h
    DESTINATION shell/adapter/include/
)

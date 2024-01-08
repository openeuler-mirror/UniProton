set(UART_SOURCE
    serial.c
    serial_io.c
    )

if(${CONFIG_CONFIG_UART_PL011})
    list(APPEND UART_SOURCE
        serial_pl011.c
    )
endif()

if(${CONFIG_CONFIG_SERIAL_RXDMA})
    list(APPEND UART_SOURCE
        serial_dma.c
    )
elseif(${CONFIG_CONFIG_SERIAL_TXDMA})
    list(APPEND UART_SOURCE
    serial_dma.c
    )
endif()

if(${CONFIG_CONFIG_16550_UART})
    list(APPEND UART_SOURCE
        uart_16550.c
    )
endif()

if(${CONFIG_CONFIG_RPMSG_UART})
    list(APPEND UART_SOURCE
        uart_rpmsg.c
    )
endif()

if(${CONFIG_CONFIG_PSEUDOTERM})
    list(APPEND UART_SOURCE
        pty.c
    )
elseif(${CONFIG_CONFIG_PSEUDOTERM_SUSV1})
    list(APPEND UART_SOURCE
        ptmx.c
    )
endif()

if(${CONFIG_CONFIG_UART_BTH4})
    list(APPEND UART_SOURCE
        uart_bth4.c
    )
endif()

add_library(nuttx_uart OBJECT ${UART_SOURCE})

target_include_directories(nuttx_uart PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

target_compile_options(nuttx_uart PUBLIC 
    -Wno-missing-field-initializers
)

list(APPEND ALL_OBJECT_LIBRARYS nuttx_uart)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
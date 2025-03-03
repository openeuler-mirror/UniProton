add_definitions(-D__UNIPROTON__)

if (${APP} STREQUAL "netTest")
    set(SRCS 
        samples/sample_common.c
        samples/sample_net.c
    )
elseif (${APP} STREQUAL "uartTest")
    set(SRCS 
        samples/sample_common.c
        samples/sample_uart.c
    )
endif()

add_library(drivers OBJECT ${SRCS})

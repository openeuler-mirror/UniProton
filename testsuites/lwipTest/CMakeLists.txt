if (NOT ${APP} STREQUAL "UniProton_lwip_demo")
    return()
endif()

set(BUILD_APP "UniProton_lwip_demo")
set(ALL_SRC
    lwip_dbg.c
    lwip_udp.c
    proxy_udp.c
)

if(${CPU_TYPE} STREQUAL "x86_64")
    add_library(lwipUdpClient OBJECT ${ALL_SRC})
endif()
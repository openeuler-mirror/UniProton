if (NOT ${APP} STREQUAL "opcuaTest")
    return()
endif()

set(BUILD_APP "opcuaTest")
set(ALL_SRC
    opcua_server.c
    opcua_client.c
)

if(${CPU_TYPE} STREQUAL "hi3093" OR ${CPU_TYPE} STREQUAL "hi3095")
    add_library(opcuaServer OBJECT ${ALL_SRC})
endif()

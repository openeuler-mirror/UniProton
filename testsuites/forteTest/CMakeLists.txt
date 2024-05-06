if (NOT ${APP} STREQUAL "UniProton_forte_demo")
    return()
endif()

set(BUILD_APP "UniProton_forte_demo")
set(ALL_SRC
    forte_Init.c
)

if(${CPU_TYPE} STREQUAL "x86_64")
    add_library(forteInit OBJECT ${ALL_SRC})
endif()
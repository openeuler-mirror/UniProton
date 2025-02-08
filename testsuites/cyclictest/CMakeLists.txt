if (NOT ${APP} STREQUAL "UniProton_cyclictest" AND NOT "${CONFIG_LOSCFG_SHELL_TEST}")
    return()
endif()

if (${APP} STREQUAL "UniProton_cyclictest")
    set(BUILD_APP "UniProton_cyclictest")
endif()

set(ALL_SRC
    cyclictest.c
)

if (${CPU_TYPE} STREQUAL "hi3093")
    add_library(cyclicTest OBJECT ${ALL_SRC})
endif()
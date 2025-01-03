if (NOT ${APP} STREQUAL "UniProton_cyclictest")
    return()
endif()

set(BUILD_APP "UniProton_cyclictest")
set(ALL_SRC
    cyclictest.c
)

if (${CPU_TYPE} STREQUAL "hi3093")
    add_library(cyclicTest OBJECT ${ALL_SRC})
endif()
if (NOT ${APP} STREQUAL "UniProton_jitter_test")
    return()
endif()

set(BUILD_APP "UniProton_jitter_test")
set(ALL_SRC
    interrupt_jitter.c
)

if (${CPU_TYPE} STREQUAL "hi3093")
    add_library(jitterTest OBJECT ${ALL_SRC})
endif()
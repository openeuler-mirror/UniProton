set(SRCS rtt_viewer.c)
if("${SIM}" STREQUAL "_SIM_")
list(APPEND SRCS
    semihosting_dbg.c)
endif()
add_library(bsp OBJECT ${SRCS})

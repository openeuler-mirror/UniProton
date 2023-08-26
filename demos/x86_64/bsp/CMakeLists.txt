set(SRCS start.S cpu_offline.c libc.c print.c)
add_library(bsp OBJECT ${SRCS})
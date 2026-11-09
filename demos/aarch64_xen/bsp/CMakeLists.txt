set(SRCS start.S cache_asm.S hwi_init.c print.c timer.c print_openamp.c)
add_library(bsp OBJECT ${SRCS})

set(SRCS start.S cache_asm.S hwi_init.c mmu.c print.c timer.c print_openamp.c)
add_library(bsp OBJECT ${SRCS})

add_subdirectory(uart)
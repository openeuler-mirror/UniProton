set(SRCS start.S cache_asm.S hwi_init.c print.c timer.c shm_pub.c)
add_library(bsp OBJECT ${SRCS})
add_compile_definitions(CONFIG_UNIPROTON_IPI)
set(SRCS boot.s trap_handler.c riscv.c uart.c)
add_library(bsp OBJECT ${SRCS})

set(SRCS boot.s misc_m/trap_handler.c misc_m/riscv.c driver/uart.c)
add_library(bsp OBJECT ${SRCS})

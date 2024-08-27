set(SRCS boot.s misc_m/trap_handler.c misc_m/riscv.c driver/sys-uart.c driver/sys-clock.c driver/sys-dram.c driver/sys-jtag.c driver/sys-spinor.c)
add_library(bsp OBJECT ${SRCS})

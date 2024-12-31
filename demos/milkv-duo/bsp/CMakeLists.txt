
set(SRCS start.S trap_handler.c hal_uart_dw.c uart.c rtos_cmdqu.c cache.c arch_sleep.c cvi_spinlock.c sleep.c)

add_library(bsp OBJECT ${SRCS})

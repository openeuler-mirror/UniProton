if(${APP} STREQUAL "rpmsglite_test_master")
set(SRCS boot_rpmsg_master.s misc_m/trap_handler.c misc_m/riscv.c driver/uart.c ipi_module/src/riscv_ipi.c)
else()
set(SRCS boot.s misc_m/trap_handler.c misc_m/riscv.c driver/uart.c ipi_module/src/riscv_ipi.c)
endif()
add_library(bsp OBJECT ${SRCS})

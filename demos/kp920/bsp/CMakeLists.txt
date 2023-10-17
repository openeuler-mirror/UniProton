set(SRCS start.S cache_asm.S hwi_init.c mmu.c print.c timer.c print_openamp.c pcie_bus_filter.c)
add_library(bsp OBJECT ${SRCS})

add_subdirectory(uart)
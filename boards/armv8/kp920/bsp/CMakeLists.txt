set(SRCS start.S cache_asm.S hwi_init.c mmu.c print.c timer.c print_openamp.c pcie_bus_filter.c)
if (${CONFIG_OS_GDB_STUB})
    list(APPEND SRCS gdbstub_cfg.c)
endif()
add_library(bsp OBJECT ${SRCS})

add_subdirectory(uart)
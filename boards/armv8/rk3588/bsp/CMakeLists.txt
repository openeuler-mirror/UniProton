set(SRCS cache_asm.S mmu.c hwi_init.c print.c timer.c)

if (${CONFIG_OS_GDB_STUB})
    list(APPEND SRCS gdbstub_cfg.c)
endif()

add_library(bsp OBJECT ${SRCS})

add_subdirectory(hal)

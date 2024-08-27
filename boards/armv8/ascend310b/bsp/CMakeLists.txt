if (${CONFIG_OS_OPTION_OPENAMP})
    set(SRCS start.S cache_asm.S hwi_init.c print.c timer.c mmu.c print_openamp.c)
    add_library(bsp OBJECT ${SRCS})
elseif(${CONFIG_GUEST_OS})
    set(SRCS
        start.S
        cache_asm.S
        hwi_init.c
        print.c
        timer.c
        shm_pub.c
        spi_1911_config.c
        spi_1911_transfer.c
        prt_gic_its.c 
        pl011.c ymodem.c
        file_transfer.c
        emmc.c
        i2c_1911_config.c
        i2c_1911_transfer.c)
    add_library(bsp OBJECT ${SRCS})
    add_compile_definitions(CONFIG_UNIPROTON_IPI)
endif()

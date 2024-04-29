add_library_ex(prt_irq.c)
if(${CONFIG_OS_OPTION_SMP})
    add_library_ex(prt_irq_minor.c)
endif()
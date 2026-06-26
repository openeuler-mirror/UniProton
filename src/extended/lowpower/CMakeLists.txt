############################## low-power framework ###########################
# Framework + register context (always compiled when the lowpower framework is on.
# Sub-modules (tickless/runstop/powermgr) are gated by their own options.)
if(${CONFIG_OS_OPTION_LOWPOWER})
    add_library_ex(prt_lowpower.c)
    add_library_ex(prt_lowpower_context.c)

    if(${CONFIG_OS_ARCH_ARMV8})
        add_subdirectory(arch/armv8)
    endif()

    if(${CONFIG_OS_OPTION_TICKLESS})
        add_subdirectory(tickless)
    endif()

    if(${CONFIG_OS_OPTION_RUNSTOP})
        add_subdirectory(runstop)
    endif()

    if(${CONFIG_OS_OPTION_POWERMGR})
        add_subdirectory(powermgr)
    endif()
endif()

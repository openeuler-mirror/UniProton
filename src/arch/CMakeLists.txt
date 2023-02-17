add_subdirectory(cpu)
if(${CONFIG_OS_ARCH_ARMV8})
    add_subdirectory(drv)
endif()

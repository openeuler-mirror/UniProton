if(${CONFIG_OS_OPTION_CPUP})
    add_subdirectory(cpup)
endif()

add_subdirectory(err)
add_subdirectory(hook)

if((${CONFIG_OS_OPTION_STACKTRACE}) AND (${CONFIG_OS_ARCH_ARMV8}))
add_subdirectory(unwind)
endif()
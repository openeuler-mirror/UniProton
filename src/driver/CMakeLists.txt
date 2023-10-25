add_subdirectory(i210)
if (${CONFIG_OS_OPTION_PCIE})
    add_subdirectory(pcie)
endif()
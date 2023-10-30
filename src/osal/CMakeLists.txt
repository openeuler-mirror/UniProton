if (${CONFIG_OS_SUPPORT_CXX})
add_subdirectory(cxx)
endif()

if (${CONFIG_OS_OPTION_LINUX})
add_subdirectory(linux)
endif()
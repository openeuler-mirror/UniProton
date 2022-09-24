##############################下一级的编译目录 ###########################
add_subdirectory(arch)
add_subdirectory(core)
add_subdirectory(mem)
add_subdirectory(om)
add_subdirectory(security)
add_subdirectory(utility)

if(${CONFIG_OS_SUPPORT_FS})
    add_subdirectory(fs)
endif()

if(${CONFIG_OS_SUPPORT_NET})
    add_subdirectory(net)
endif()



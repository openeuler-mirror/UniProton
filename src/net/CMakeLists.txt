##############################下一级的编译目录 ###########################
if(${CONFIG_OS_SUPPORT_NET})
add_subdirectory(lwip-2.1)
endif()

if (${CONFIG_OS_SUPPORT_ETHERCAT})
add_subdirectory(ethercat)
endif()

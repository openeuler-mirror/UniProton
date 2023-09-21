##############################下一级的编译目录 ###########################
if(${CONFIG_OS_SUPPORT_NET})
add_subdirectory(lwip-2.1)
endif()

if (${PLAM_TYPE} STREQUAL "x86_64")
add_subdirectory(ethercat)
endif()

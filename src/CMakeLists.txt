##############################下一级的编译目录 ###########################
add_subdirectory(arch)
add_subdirectory(core)
add_subdirectory(mem)
add_subdirectory(om)
add_subdirectory(security)
add_subdirectory(utility)
add_subdirectory(kal)
add_subdirectory(libc)
add_subdirectory(osal)

set(UNIPROTON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(fs)
add_subdirectory(drivers)

add_subdirectory(net)

if(${CONFIG_OS_GDB_STUB})
    add_subdirectory(component/gdbstub)
endif()

if(${CONFIG_OS_SUPPORT_CXX})
    add_subdirectory(component/cxx)
endif()

if(${CONFIG_LOSCFG_SHELL})
    add_subdirectory(shell)
endif()

if((${CONFIG_OS_SUPPORT_LIBXML2}) AND (EXISTS ${UNIPROTON_SRC_DIR}/component/libxml2))
    add_subdirectory(component/libxml2)
endif()

if(${CONFIG_OS_SUPPORT_RPMSGLITE})
    add_subdirectory(component/rpmsg-lite)
endif()

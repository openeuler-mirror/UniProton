cmake_minimum_required(VERSION 3.14.1) #cmake最低版本要求
# 下载lwip源码到lwip文件夹
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lwip)
    message("--- Download lwip")
    include(FetchContent)
    FetchContent_Declare(
        lwip
        URL https://gitee.com/src-openeuler/lwip/raw/openEuler-22.03-LTS-SP3-release/lwip-2.1.3.zip
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip
        PATCH_COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && patch -p1 -d lwip < ${CMAKE_CURRENT_SOURCE_DIR}/UniProton-patch-for-lwip.patch
    )
    FetchContent_Populate(lwip)
    FetchContent_GetProperties(lwip)
    message("--- End download lwip path: ${lwip_SOURCE_DIR}")
endif()

##############################下一级的编译目录 ###########################
if (${CONFIG_OS_SUPPORT_NET})
add_subdirectory(lwip)
endif()

if (${CONFIG_OS_SUPPORT_IGH_ETHERCAT})
add_subdirectory(ethercat)
endif()

if (${CONFIG_OS_SUPPORT_SOEM})
add_subdirectory(soem)
endif()

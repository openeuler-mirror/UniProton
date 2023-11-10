cmake_minimum_required(VERSION 3.14.1) #cmake最低版本要求
# 下载ff15源码到ff15文件夹
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ff15)
    message("--- Download ff15")
    include(FetchContent)
    FetchContent_Declare(
        ff15
        URL http://elm-chan.org/fsw/ff/arc/ff15.zip
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ff15
    )
    FetchContent_Populate(ff15)
    FetchContent_GetProperties(ff15)
    message("--- End download ff15 path: ${lwip_SOURCE_DIR}")
endif()
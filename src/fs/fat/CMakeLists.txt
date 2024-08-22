cmake_minimum_required(VERSION 3.14.1) #cmake最低版本要求

if(NOT ${CONFIG_OS_OPTION_DRIVER})
    RETURN()
endif()

# 下载ff15源码到ff15文件夹
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ff15)
    message("--- Download ff15")
    include(FetchContent)
    FetchContent_Declare(
        ff15
        GIT_REPOSITORY https://gitee.com/open-instrument/fatfs.git
        GIT_TAG b11f08931929e5f2f1fe8a3a2c0bd16d222b5625
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ff15
        PATCH_COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && patch -p1 -d ff15 < ${CMAKE_CURRENT_SOURCE_DIR}/0000-adapter_fix_for_ff15.patch
    )
    FetchContent_Populate(ff15)
    FetchContent_GetProperties(ff15)
    message("--- End download ff15 path: ${lwip_SOURCE_DIR}")
endif()

set(FAT_SOURCE diskio.c
               fat_vfsops.c
               ff15/source/ff.c
               ff15/source/ffsystem.c 
               ff15/source/ffunicode.c
)

add_library(ff_fat OBJECT ${FAT_SOURCE})

target_include_directories(ff_fat PUBLIC 
    ff15/source/
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/include/nuttx/drivers
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS ff_fat)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
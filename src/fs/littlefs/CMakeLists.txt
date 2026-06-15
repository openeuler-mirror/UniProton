cmake_minimum_required(VERSION 3.14.1)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/littlefs)
    message("--- Download littlefs")
    include(FetchContent)
    FetchContent_Declare(
        littlefs_src
        GIT_REPOSITORY https://github.com/littlefs-project/littlefs.git
        GIT_TAG v2.9.3
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/littlefs
    )
    FetchContent_Populate(littlefs_src)
    message("--- End download littlefs")
endif()

set(LITTLEFS_SOURCE
    littlefs_vfsops.c
    littlefs/lfs.c
    littlefs/lfs_util.c
)

add_library(littlefs OBJECT ${LITTLEFS_SOURCE})

target_include_directories(littlefs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/littlefs
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS littlefs)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

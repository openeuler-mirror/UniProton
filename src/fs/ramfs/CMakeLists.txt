cmake_minimum_required(VERSION 3.14.1)

add_library(ramfs OBJECT ramfs_vfsops.c)

target_include_directories(ramfs PUBLIC
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS ramfs)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

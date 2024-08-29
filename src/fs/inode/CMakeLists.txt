file(GLOB_RECURSE INODE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(base_inode OBJECT ${INODE_SOURCE})

target_include_directories(base_inode PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/vfs
    ${FS_BASE_DIR}/
    ${HOME_PATH}/src/libc/litelibc/internal/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/src/libc/musl/src/include
    ${HOME_PATH}/src/libc/musl/include
)

target_compile_options(base_inode PUBLIC 
        -Wno-unused-value
        )

list(APPEND ALL_OBJECT_LIBRARYS base_inode)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
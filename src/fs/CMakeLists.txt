if(NOT "${CONFIG_OS_OPTION_NUTTX_VFS}")
    RETURN()
endif()

add_library(fs_init OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/fs_initialize.c)

set(FS_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(fs_init PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/vfs
    ${FS_BASE_DIR}/
)

add_custom_target(copy_include_file ALL)
add_custom_command(TARGET copy_include_file
                   POST_BUILD
                   COMMAND mkdir -p ${FS_BASE_DIR}/include/inode
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/inode/inode.h ${FS_BASE_DIR}/include/inode/
)

list(APPEND ALL_OBJECT_LIBRARYS fs_init)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

add_subdirectory(adapter)
add_subdirectory(inode)
add_subdirectory(mount)
add_subdirectory(vfs)
add_subdirectory(sys)
add_subdirectory(fat)

install(DIRECTORY
    ${FS_BASE_DIR}/include
    DESTINATION drivers/
)

if(${CONFIG_OS_OPTION_DRIVER})
    add_subdirectory(driver)
endif()
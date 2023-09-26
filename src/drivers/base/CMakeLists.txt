add_library(base_init OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/fs_initialize.c)

target_include_directories(base_init PUBLIC 
    ${DRIVERS_BASE_DIR}/include
    ${DRIVERS_BASE_DIR}/base
)

add_custom_target(copy_include_file ALL)
add_custom_command(TARGET copy_include_file
                   POST_BUILD
                   COMMAND mkdir -p ${DRIVERS_BASE_DIR}/include/inode
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/inode/inode.h ${DRIVERS_BASE_DIR}/include/inode/
)

list(APPEND ALL_OBJECT_LIBRARYS base_init)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

add_subdirectory(inode)
add_subdirectory(mount)
add_subdirectory(vfs)
add_subdirectory(libs)

if(${CONFIG_OS_OPTION_DRIVER})
    add_subdirectory(driver)
endif()
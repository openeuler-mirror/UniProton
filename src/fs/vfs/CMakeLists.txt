file(GLOB_RECURSE VFS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)


if(NOT "${CONFIG_CONFIG_PSEUDOFS_SOFTLINKS}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_link.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_symlink.c 
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_readlink.c
    )
endif()

if(NOT "${CONFIG_OS_OPTION_PROXY}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_proxy.c
    )
endif()

# Stream support

if(NOT "${CONFIG_CONFIG_FILE_STREAM}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_fdopen.c
    )
endif()

# Support for eventfd

if(NOT "${CONFIG_CONFIG_EVENT_FD}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_eventfd.c
    )
endif()

# Support for timerfd

if(NOT "${CONFIG_CONFIG_TIMER_FD}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_timerfd.c
    )
endif()

# Support for signalfd

if(NOT "${CONFIG_CONFIG_SIGNAL_FD}")
    list(REMOVE_ITEM VFS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/fs_signalfd.c
    )
endif()

add_library(base_vfs OBJECT ${VFS_SOURCE})

target_compile_options(base_vfs PUBLIC 
    -Wno-missing-field-initializers
    )

target_include_directories(base_vfs PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS base_vfs)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
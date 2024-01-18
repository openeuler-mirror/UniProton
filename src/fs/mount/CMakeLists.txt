if(NOT "${CONFIG_OS_OPTION_DRIVER}")
    RETURN()
endif()

if ("${CONFIG_CONFIG_DISABLE_MOUNTPOINT}")
    RETURN()
endif()

set(MOUNT_SOURCE
    fs_mount.c
    fs_umount2.c
    fs_foreachmountpoint.c
    )

if("${CONFIG_CONFIG_FS_AUTOMOUNTER}")
    list(APPEND MOUNT_SOURCE
        fs_automount.c
    )
endif()

if("${CONFIG_CONFIG_FS_PROCFS}" AND (NOT "${CONFIG_CONFIG_FS_PROCFS_EXCLUDE_MOUNT}"))
    list(APPEND MOUNT_SOURCE 
        fs_procfs_mount.c 
        fs_gettype.c
    )
endif()

add_library(base_mount OBJECT ${MOUNT_SOURCE})

target_include_directories(base_mount PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/include/nuttx/drivers
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS base_mount)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
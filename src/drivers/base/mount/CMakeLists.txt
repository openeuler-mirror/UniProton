if(NOT ${CONFIG_OS_OPTION_DRIVER})
    RETURN()
endif()

set(MOUNT_SOURCE
    fs_mount.c
    fs_umount2.c
    fs_foreachmountpoint.c
    )

if(NOT ${CONFIG_CONFIG_DISABLE_MOUNTPOINT})
    list(APPEND MOUNT_SOURCE
        fs_automount.c
    )
endif()

if(${CONFIG_CONFIG_FS_PROCFS} AND (NOT ${CONFIG_CONFIG_FS_PROCFS_EXCLUDE_MOUNT}))
    list(APPEND MOUNT_SOURCE 
        fs_procfs_mount.c 
        fs_gettype.c
    )
endif()

add_library(base_mount OBJECT ${MOUNT_SOURCE})

target_include_directories(base_mount PUBLIC 
    ${DRIVERS_BASE_DIR}/include
    ${DRIVERS_BASE_DIR}/include/nuttx/drivers
    ${DRIVERS_BASE_DIR}/base
)

list(APPEND ALL_OBJECT_LIBRARYS base_mount)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
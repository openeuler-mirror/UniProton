if(NOT "${CONFIG_OS_OPTION_DRIVER}")
    RETURN()
endif()

set(DRIVER_SOURCE fs_registerdriver.c fs_unregisterdriver.c)

if(NOT "${CONFIG_CONFIG_DISABLE_MOUNTPOINT}")
    list(APPEND DRIVER_SOURCE
        fs_registerblockdriver.c 
        fs_unregisterblockdriver.c
        fs_findblockdriver.c 
        fs_openblockdriver.c 
        fs_closeblockdriver.c
        fs_blockpartition.c 
        fs_findmtddriver.c 
        fs_closemtddriver.c
    )
endif()

if(${CONFIG_CONFIG_MTD})
    list(APPEND DRIVER_SOURCE
        fs_registermtddriver.c 
        fs_unregistermtddriver.c
        fs_mtdproxy.c
    )
    if(${CONFIG_CONFIG_MTD_PARTITION})
        list(APPEND DRIVER_SOURCE fs_mtdpartition.c)
    endif()
endif()

if(${CONFIG_CONFIG_BCH})
    if(NOT "${CONFIG_DISABLE_PSEUDOFS_OPERATIONS}")
        list(APPEND DRIVER_SOURCE fs_blockproxy.c)
    endif()
endif()

add_library(base_driver OBJECT ${DRIVER_SOURCE})

target_include_directories(base_driver PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS base_driver)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
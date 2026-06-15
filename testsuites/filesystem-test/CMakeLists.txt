if (${APP} STREQUAL "fs")
    set(ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/filesystem_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/fatfs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ramfs/ramfs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/spiffs/spiffs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/littlefs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/devfs/devfs.c
    )
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(filesystemTest OBJECT ${ALL_SRC})
    target_include_directories(filesystemTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/demos/sd3403/include/libc/include
        ${HOME_PATH}/output/libc/include
        ${HOME_PATH}/src/include/uapi
        ${HOME_PATH}/src/fs/include
        ${HOME_PATH}/platform/libboundscheck/include
        ${HOME_PATH}/demos/sd3403/config
        ${HOME_PATH}/demos/sd3403/include
    )
endif()

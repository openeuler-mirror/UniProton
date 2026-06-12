if (${APP} STREQUAL "fatfs")
    set(ALL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/fatfs.c)
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(filesystemTest OBJECT ${ALL_SRC})
endif()

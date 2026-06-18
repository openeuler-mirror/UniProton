cmake_minimum_required(VERSION 3.14.1)

if(NOT "${CONFIG_OS_SUPPORT_ZLIB}" STREQUAL "y")
    return()
endif()

set(ZLIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zutil.c
)

add_library(zlib_component OBJECT ${ZLIB_SRC})

list(APPEND ALL_OBJECT_LIBRARYS zlib_component)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

add_library(zlib_config INTERFACE)

target_include_directories(zlib_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HOME_PATH}/src/include/uapi
    ${HOME_PATH}/demos/sd3403/config
    ${HOME_PATH}/demos/sd3403/include
    ${HOME_PATH}/demos/sd3403/include/libc/include
    ${HOME_PATH}/output/libc/include
)

target_link_libraries(zlib_component PUBLIC zlib_config)

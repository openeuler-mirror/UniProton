cmake_minimum_required(VERSION 3.14.1)

set(SPIFFS_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core)
if(NOT EXISTS ${SPIFFS_CORE_DIR}/spiffs_hydrogen.c)
    message(FATAL_ERROR "SPIFFS core source not found: ${SPIFFS_CORE_DIR}")
endif()
set(SPIFFS_SOURCE
    spiffs_vfsops.c
    ${SPIFFS_CORE_DIR}/spiffs_hydrogen.c
    ${SPIFFS_CORE_DIR}/spiffs_cache.c
    ${SPIFFS_CORE_DIR}/spiffs_check.c
    ${SPIFFS_CORE_DIR}/spiffs_gc.c
    ${SPIFFS_CORE_DIR}/spiffs_nucleus.c
)

add_library(spiffs OBJECT ${SPIFFS_SOURCE})

target_include_directories(spiffs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SPIFFS_CORE_DIR}
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
)

list(APPEND ALL_OBJECT_LIBRARYS spiffs)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

file(GLOB_RECURSE ARMV8_CACHE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.S)

add_library(armv8_cache OBJECT ${ARMV8_CACHE_SRC})

target_include_directories(armv8_cache PUBLIC
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS armv8_cache)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
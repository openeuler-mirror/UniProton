file(GLOB_RECURSE UTILITY_LOG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(utility_log OBJECT ${UTILITY_LOG_SRC})

target_include_directories(utility_log PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS utility_log)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
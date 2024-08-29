
file(GLOB_RECURSE SECURITY_RND_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(security_rnd OBJECT ${SECURITY_RND_SRC})

target_include_directories(security_rnd PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS security_rnd)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
file(GLOB_RECURSE OM_HOOK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(om_hook OBJECT ${OM_HOOK_SRC})

target_include_directories(om_hook PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS om_hook)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

file(GLOB_RECURSE OM_UNWIND_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(om_unwind OBJECT ${OM_UNWIND_SRC})

target_include_directories(om_unwind PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS om_unwind)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
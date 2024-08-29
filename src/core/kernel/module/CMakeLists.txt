file(GLOB_RECURSE KERNEL_MODULE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(kernel_module OBJECT ${KERNEL_MODULE_SRC})

target_include_directories(kernel_module PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS kernel_module)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
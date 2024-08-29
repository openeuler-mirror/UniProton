file(GLOB_RECURSE KERNEL_KEXC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(kernel_kexc OBJECT ${KERNEL_KEXC_SRC})

target_include_directories(kernel_kexc PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS kernel_kexc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

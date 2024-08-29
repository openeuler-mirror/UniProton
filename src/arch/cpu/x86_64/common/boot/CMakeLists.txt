file(GLOB_RECURSE X86_BOOT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(x86_boot OBJECT ${X86_BOOT_SRC})

target_include_directories(x86_boot PUBLIC
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS x86_boot)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
file(GLOB_RECURSE X86_EXC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(x86_exc OBJECT ${X86_EXC_SRC})

target_include_directories(x86_exc PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS x86_exc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
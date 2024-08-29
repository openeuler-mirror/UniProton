add_subdirectory(boot)
add_subdirectory(hwi)
add_subdirectory(exc)

set(X86_COMMON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/prt_port.c)

add_library(x86_common OBJECT ${X86_COMMON_SRC})

target_include_directories(x86_common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS x86_common)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
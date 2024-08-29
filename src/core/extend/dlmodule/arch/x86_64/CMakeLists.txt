set(DYNMODULE_X86
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_dynmodule_x86_64.c
)

add_library(dynmodule_x86 OBJECT ${DYNMODULE_X86})

target_include_directories(dynmodule_x86 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS dynmodule_x86)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

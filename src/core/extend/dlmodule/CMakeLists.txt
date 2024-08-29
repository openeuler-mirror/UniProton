add_subdirectory(arch)
set(DLMODULE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_dynmodule_symbol.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_dynmodule.c
)

# file(GLOB_RECURSE ARMV8_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_library(dlmodule_src OBJECT ${DLMODULE_SRC})

target_include_directories(dlmodule_src PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS dlmodule_src)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

set(DYNMODULE_ARMV8
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_dynmodule_armv8.c
)

# file(GLOB_RECURSE ARMV8_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_library(dynmodule_armv8 OBJECT ${DYNMODULE_ARMV8})

target_include_directories(dynmodule_armv8 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS dynmodule_armv8)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)


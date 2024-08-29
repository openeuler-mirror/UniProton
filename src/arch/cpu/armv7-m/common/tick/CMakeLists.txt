file(GLOB_RECURSE ARMV7_TICK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(armv7_tick OBJECT ${ARMV7_TICK_SRC})

target_include_directories(armv7_tick PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS armv7_tick)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
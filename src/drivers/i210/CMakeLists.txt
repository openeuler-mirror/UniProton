file(GLOB_RECURSE DRIVERS_I210_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(drivers_i210 OBJECT ${DRIVERS_I210_SRC})

target_include_directories(drivers_i210 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS drivers_i210)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

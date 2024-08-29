file(GLOB_RECURSE RISCV64_COMMON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(riscv64_common OBJECT ${RISCV64_COMMON_SRC})

target_include_directories(riscv64_common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS riscv64_common)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
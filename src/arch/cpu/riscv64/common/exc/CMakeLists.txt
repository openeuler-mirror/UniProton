file(GLOB_RECURSE RISCV64_EXC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(riscv64_exc OBJECT ${RISCV64_EXC_SRC})

target_include_directories(riscv64_exc PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS riscv64_exc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
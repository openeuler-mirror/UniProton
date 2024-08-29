file(GLOB_RECURSE RISCV64_BOOT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(riscv64_boot OBJECT ${RISCV64_BOOT_SRC})

target_include_directories(riscv64_boot PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS riscv64_boot)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
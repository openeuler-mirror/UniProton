file(GLOB_RECURSE KERNEL_SPINLOCK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(kernel_spinlock OBJECT ${KERNEL_SPINLOCK_SRC})

target_include_directories(kernel_spinlock PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS kernel_spinlock)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
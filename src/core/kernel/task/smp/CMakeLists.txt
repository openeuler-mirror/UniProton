file(GLOB_RECURSE TASK_SMP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(task_smp OBJECT ${TASK_SMP_SRC})

target_include_directories(task_smp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS task_smp)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
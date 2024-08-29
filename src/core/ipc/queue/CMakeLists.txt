file(GLOB_RECURSE IPC_QUEUE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(ipc_queue OBJECT ${IPC_QUEUE_SRC})

target_include_directories(ipc_queue PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS ipc_queue)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

file(GLOB_RECURSE IPC_SIGNAL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(ipc_signal OBJECT ${IPC_SIGNAL_SRC})

target_include_directories(ipc_signal PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS ipc_signal)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
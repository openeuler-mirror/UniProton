if(NOT ${CONFIG_OS_OPTION_POSIX})
    RETURN()
endif()

message("======== Compile KAL for POSIX dir  ========")

file(GLOB_RECURSE KAL_SOURCE src/*.c)

add_library(kallibc OBJECT ${KAL_SOURCE})

target_include_directories(kallibc PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
        ${HOME_PATH}/src/core/ipc/rwlock
)

list(APPEND ALL_OBJECT_LIBRARYS kallibc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
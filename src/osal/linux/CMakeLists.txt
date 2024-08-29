message(${CMAKE_SYSTEM_PROCESSOR})

file(GLOB_RECURSE LINUX_SOURCE
    src/*.c
)

add_library(linuxlib OBJECT ${LINUX_SOURCE})

target_include_directories(linuxlib PUBLIC
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
        ${HOME_PATH}/src/core/ipc/rwlock
        ${HOME_PATH}/src/drivers/i210
        ${HOME_PATH}/src/core/kernel/task
)

target_compile_options(linuxlib PUBLIC
        -Wno-bad-function-cast
        -Wno-unused-value
        -Wno-switch-default
        -Wno-float-equal
        -Wno-unused-but-set-variable
        -Wno-discarded-qualifiers
        -Wno-implicit-fallthrough
        -Wno-cast-function-type
        -Werror=incompatible-pointer-types
        -Werror=missing-prototypes
        -Werror=missing-parameter-type
        -Werror=missing-declarations
        -Werror=implicit-function-declaration
        -Werror=return-type
        )

list(APPEND ALL_OBJECT_LIBRARYS linuxlib)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include_self_def/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/include_self_def/asm
    DESTINATION linux/include/
)
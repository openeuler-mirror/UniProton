add_subdirectory(boot)
add_subdirectory(exc)
add_subdirectory(hwi)
add_subdirectory(tick)

set(ARMV7_COMMON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/prt_port.c)

add_library(armv7_common OBJECT ${ARMV7_COMMON_SRC})

target_include_directories(armv7_common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS armv7_common)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
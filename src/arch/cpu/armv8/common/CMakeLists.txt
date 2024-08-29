add_subdirectory(boot)
add_subdirectory(cache)
add_subdirectory(exc)
add_subdirectory(hwi)
if(${CONFIG_OS_OPTION_STACKTRACE})
add_subdirectory(stacktrace)
endif()

set(ARMV8_COMMON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/prt_port.c)

add_library(armv8_common OBJECT ${ARMV8_COMMON_SRC})

target_include_directories(armv8_common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS armv8_common)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
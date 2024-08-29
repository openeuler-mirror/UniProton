file(GLOB_RECURSE TIMER_SWTMR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(timer_swtmr OBJECT ${TIMER_SWTMR_SRC})

target_include_directories(timer_swtmr PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS timer_swtmr)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
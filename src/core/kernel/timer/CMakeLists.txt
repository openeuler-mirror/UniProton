if(${CONFIG_INTERNAL_OS_SWTMR})
    add_subdirectory(swtmr)
endif()

set(KERNEL_TIMER_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_timer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_timer_minor.c
)

add_library(kernel_timer OBJECT ${KERNEL_TIMER_SRC})

target_include_directories(kernel_timer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS kernel_timer)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)


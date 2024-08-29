file(GLOB_RECURSE TASK_AMP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

if(NOT "${CONFIG_OS_OPTION_POWEROFF}")
    list(REMOVE_ITEM TASK_AMP_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_amp_psci.c
    )
endif()

add_library(task_amp OBJECT ${TASK_AMP_SRC})

target_include_directories(task_amp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS task_amp)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
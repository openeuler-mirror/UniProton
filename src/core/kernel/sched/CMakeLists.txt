if(${CONFIG_OS_OPTION_SMP})
    set(KERNEL_SCHED_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_rt_single.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_sched.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_sched_class.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_sched_init.c)
else()
    set(KERNEL_SCHED_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_sched_single.c)
endif()

add_library(sched_src OBJECT ${KERNEL_SCHED_SRC})

target_include_directories(sched_src PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS sched_src)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)



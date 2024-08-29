file(GLOB_RECURSE OM_CPUP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

if(NOT "${CONFIG_INTERNAL_OS_CPUP_THREAD}")
    list(REMOVE_ITEM OM_CPUP_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_thread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_thread_init.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_thread_64.c
    )
endif()

if(NOT "${CONFIG_OS_OPTION_CPUP_WARN}")
    list(REMOVE_ITEM OM_CPUP_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_warn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_thread_init.c
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_cpup_thread_64.c
    )
endif()

add_library(om_cpup OBJECT ${OM_CPUP_SRC})

target_include_directories(om_cpup PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS om_cpup)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

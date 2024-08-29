file(GLOB_RECURSE OSAL_CXX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(osal_cxx OBJECT ${OSAL_CXX_SRC})

target_include_directories(osal_cxx PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS osal_cxx)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
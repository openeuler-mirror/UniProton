file(GLOB_RECURSE PIPES_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(nuttx_pipes OBJECT ${PIPES_SOURCE})

target_include_directories(nuttx_pipes PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
    ${HOME_PATH}/src/libc/litelibc/internal/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/src/libc/musl/src/include
    ${HOME_PATH}/src/libc/musl/include
)

target_compile_options(nuttx_pipes PUBLIC 
    -Wno-discarded-qualifiers
    -Wno-address
    -Wno-missing-field-initializers
)

list(APPEND ALL_OBJECT_LIBRARYS nuttx_pipes)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
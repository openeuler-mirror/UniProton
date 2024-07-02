file(GLOB_RECURSE SYS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(base_sys OBJECT ${SYS_SOURCE})

target_include_directories(base_sys PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/vfs
    ${FS_BASE_DIR}/
)

target_compile_options(base_sys PUBLIC 
    -Wno-discarded-qualifiers
    -Wno-address
)

list(APPEND ALL_OBJECT_LIBRARYS base_sys)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
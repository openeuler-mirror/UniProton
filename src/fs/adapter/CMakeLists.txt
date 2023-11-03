file(GLOB_RECURSE ADAPTER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(fs_adapter OBJECT ${ADAPTER_SOURCE})

target_include_directories(fs_adapter PUBLIC 
    ${FS_BASE_DIR}/include
    ${FS_BASE_DIR}/
    ${UNIPROTON_SRC_DIR}/core/kernel/irq
)

target_compile_options(fs_adapter PUBLIC 
        -Wno-cast-function-type
)

list(APPEND ALL_OBJECT_LIBRARYS fs_adapter)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
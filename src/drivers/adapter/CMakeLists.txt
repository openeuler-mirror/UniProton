file(GLOB_RECURSE ADAPTER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(base_adapter OBJECT ${ADAPTER_SOURCE})

target_include_directories(base_adapter PUBLIC 
    ${DRIVERS_BASE_DIR}/include
    ${DRIVERS_BASE_DIR}/base
    ${UNIPROTON_SRC_DIR}/core/kernel/irq
)

target_compile_options(base_adapter PUBLIC 
        -Wno-cast-function-type
)

list(APPEND ALL_OBJECT_LIBRARYS base_adapter)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
set(MTD_SOURCE ftl.c
               mtd_progmem.c
               mtd_partition.c
               )

add_library(nuttx_mtd OBJECT ${MTD_SOURCE})

target_compile_options(nuttx_mtd PUBLIC 
    -Wno-missing-field-initializers
    -Wno-bad-function-cast
)

target_include_directories(nuttx_mtd PUBLIC 
    ${HOME_PATH}/src/libc/litelibc/internal/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/src/libc/musl/src/include
    ${HOME_PATH}/src/libc/musl/include
)


list(APPEND ALL_OBJECT_LIBRARYS nuttx_mtd)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
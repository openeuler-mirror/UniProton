set(MTD_SOURCE ftl.c
               mtd_progmem.c
               mtd_partition.c
               )

add_library(nuttx_mtd OBJECT ${MTD_SOURCE})

target_compile_options(nuttx_mtd PUBLIC 
    -Wno-missing-field-initializers
    -Wno-bad-function-cast
)

list(APPEND ALL_OBJECT_LIBRARYS nuttx_mtd)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
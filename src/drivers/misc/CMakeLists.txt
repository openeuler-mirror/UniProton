set(MISC_SOURCE rwbuffer.c
               )

add_library(nuttx_misc OBJECT ${MISC_SOURCE})

target_compile_options(nuttx_misc PUBLIC 
    -Wno-missing-field-initializers
    -Wno-bad-function-cast
)

list(APPEND ALL_OBJECT_LIBRARYS nuttx_misc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
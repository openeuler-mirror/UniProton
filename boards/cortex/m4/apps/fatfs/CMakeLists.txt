set(SRCS main.c)
add_library(apps OBJECT ${SRCS})

target_compile_options(apps PUBLIC 
    -Wno-implicit-function-declaration
    -Wno-nested-externs
    -Wno-discarded-qualifiers
    -Wno-missing-field-initializers
)
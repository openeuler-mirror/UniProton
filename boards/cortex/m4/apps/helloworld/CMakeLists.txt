set(SRCS main.c)
add_library(apps OBJECT ${SRCS})

target_compile_options(apps PUBLIC 
    -Wno-discarded-qualifiers
    -Wno-missing-field-initializers
    -Wno-unused-variable
)
set(SRCS system/src/error_code.cpp filesystem/src/path.cpp filesystem/src/operations.cpp)
add_library(boost OBJECT ${SRCS})

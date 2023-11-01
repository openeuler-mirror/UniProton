set(SRC rpmsg_backend.c rpmsg_service.c example_default.c main.c)
add_library(rpmsg OBJECT ${SRC})
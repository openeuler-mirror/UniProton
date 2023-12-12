set (SRC main.c rpmsg_backend.c rpmsg_service.c example_default.c)
add_library(openamp OBJECT ${SRC})

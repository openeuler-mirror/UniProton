set(SRC main.c rpmsg_backend.c rpmsg_service.c)

add_library(openamp OBJECT ${SRC})

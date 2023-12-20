set (SRC main.c rpmsg_backend.c rpmsg_service.c example_muti.c test_stub.c)
add_library(openamp OBJECT ${SRC})

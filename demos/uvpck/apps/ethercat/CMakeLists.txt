set (SRC ethercat_igh.c main.c rpmsg_backend.c rpmsg_service.c)
add_library(ethercat OBJECT ${SRC})
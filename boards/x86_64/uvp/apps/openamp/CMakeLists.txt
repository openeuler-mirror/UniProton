set (SRC main.c rpmsg_backend.c rpmsg_service.c example_muti.c i40ecs_dbg.c test_udp.c)
add_library(openamp OBJECT ${SRC})

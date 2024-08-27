set(SRC main.c ivshmem_demo.c lua_stub.c rpmsg_backend.c rpmsg_service.c)
add_library(ivshmem OBJECT ${SRC})

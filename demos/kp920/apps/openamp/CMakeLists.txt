set(SRC rpmsg_backend.c rpmsg_service.c main.c pcie_demo.c)
add_library(rpmsg OBJECT ${SRC})

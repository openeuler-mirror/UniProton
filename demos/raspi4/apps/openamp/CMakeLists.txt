if(${CONFIG_OS_OPTION_RSC_TABLE})
    set(SRC resource_table.c rpmsg_backend_rsc_table.c rpmsg_service.c main.c)
else()
    set(SRC rpmsg_backend.c rpmsg_service.c main.c)
endif()

add_library(rpmsg OBJECT ${SRC})

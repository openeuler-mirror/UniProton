set(SRC rpmsg_backend.c rpmsg_service.c example_default.c main.c)

if (${CONFIG_OS_OPTION_RSC_TABLE})
    list(APPEND SRC resource_table.c)
endif()

add_library(rpmsg OBJECT ${SRC})

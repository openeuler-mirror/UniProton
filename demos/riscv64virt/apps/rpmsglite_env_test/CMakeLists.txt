set(SRCS main.c rpmsg_common.c rpmsg_master_epts.c rpmsg_remote_epts.c)
include_directories(
    ../../../../src/component/rpmsg-lite/include/
)
add_library(app OBJECT ${SRCS})

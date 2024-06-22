add_definitions(-DCONFIG_SHMEM_SELF)
if(${APP} STREQUAL "rpmsglite_test_master")
    set(SRCS main.c rpmsg_common.c rpmsg_master_epts.c rpmsg_platform_riscv.c)
elseif(${APP} STREQUAL "rpmsglite_test_slave")
    set(SRCS main.c rpmsg_common.c rpmsg_remote_epts.c rpmsg_platform_riscv.c)
else()
    set(SRCS main.c rpmsg_common.c rpmsg_master_epts.c rpmsg_remote_epts.c rpmsg_platform_riscv.c)
endif()

include_directories(
    ../../../../src/component/rpmsg-lite/include/
)
add_library(app OBJECT ${SRCS})

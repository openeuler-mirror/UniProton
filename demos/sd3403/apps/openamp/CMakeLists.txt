set(SRC main.c)
add_library(rpmsg OBJECT ${SRC})

# openarm app: the two Direct IPC channels and the dual-arm control task.
add_library(openarm_ipc OBJECT direct_ipc.c)

# Dual-arm admittance backend: two independent 7-joint virtual-clutch
# controllers with URDF-verified per-arm parameters, plus the grippers.
add_library(openarm_control_dual OBJECT openarm_control_lib_dual.cc)
# Eigen, pinocchio and Boost come from CONFIG_OS_SUPPORT_EIGEN/PINOCCHIO.
set_source_files_properties(openarm_control_lib_dual.cc PROPERTIES COMPILE_OPTIONS "-O2")

# openarm app: the CAN-FD gateway (PL022 PIO SPI, two MCP2518FDs, one
# state machine per arm). It polls MMIO on the 1 kHz beat, so it is built
# -O2 like the control libraries.
add_library(openarm_gateway OBJECT canfd_gw_spi.c canfd_gw_can.c canfd_gw_gateway.c openarm_can.c)
target_compile_options(openarm_gateway PRIVATE -O2)

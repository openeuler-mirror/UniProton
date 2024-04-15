################交叉编译#####################
set(CPU_TYPE "$ENV{CPU_TYPE}" ) 
set(PLAM_TYPE "$ENV{PLAM_TYPE}" ) 
set(LIB_TYPE "$ENV{LIB_TYPE}"  ) 
set(SYSTEM "$ENV{SYSTEM}" )
set(CORE "$ENV{CORE}" )
set(LIB_RUN_TYPE "$ENV{LIB_RUN_TYPE}" ) 
set(BUILD_DIR "$ENV{BUILD_TMP_DIR}" ) #version id
set(OBJCOPY_PATH "$ENV{OBJCOPY_PATH}" ) #OBJCOPY_PATH
set(COMPILE_MODE "$ENV{COMPILE_MODE}" )
set(HOME_PATH "$ENV{HOME_PATH}" )
set(COMPILE_OPTION "$ENV{COMPILE_OPTION}" )
set(RPROTON_INSTALL_FILE_OPTION "$ENV{RPROTON_INSTALL_FILE_OPTION}" )##RPROTON_INSTALL_FILE_OPTION="SUPER_BUILD";


if (${CPU_TYPE} STREQUAL "m4")
#### 统一告警选项,请审慎增删
set(STRONG_COMPILE_WARING_FLAG "-Wunused -Wredundant-decls -Wfloat-conversion -Wwrite-strings -Wunused-macros -Wswitch-default -Wshift-overflow=2 -Wnested-externs -Wmissing-include-dirs -Wlogical-op -Wjump-misses-init -Wformat-security -Wvla -Wframe-larger-than=4096 -Wduplicated-cond  -Wdisabled-optimization -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wpointer-arith -Wshift-negative-value -Wsign-compare -Wtype-limits -Wcast-qual -Wundef -Wbad-function-cast -Wold-style-definition -Wpacked -Wstrict-prototypes -Wstack-usage=2048")
set(COMPILE_WARING_FLAG " -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal -Wshadow -Wtrampolines -Wdate-time ")## -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal 
include(${HOME_PATH}/cmake/tool_chain/uniproton_tool_chain_gcc.cmake)
endif()

if (${CPU_TYPE} STREQUAL "raspi4" OR ${CPU_TYPE} STREQUAL "hi3093" OR ${CPU_TYPE} STREQUAL "atlasa1" OR ${CPU_TYPE} STREQUAL "kp920" OR ${CPU_TYPE} STREQUAL "rk3568_jailhouse" OR ${CPU_TYPE} STREQUAL "rk3588" OR ${CPU_TYPE} STREQUAL "ascend310b" OR ${CPU_TYPE} STREQUAL "e2000q")
#### 统一告警选项,请审慎增删
set(STRONG_COMPILE_WARING_FLAG "-Wunused -Wredundant-decls -Wfloat-conversion -Wwrite-strings -Wunused-macros -Wswitch-default -Wshift-overflow=2 -Wnested-externs -Wmissing-include-dirs -Wlogical-op -Wjump-misses-init -Wformat-security -Wvla -Wframe-larger-than=4096 -Wduplicated-cond  -Wdisabled-optimization -Wduplicated-branches -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wpointer-arith -Wshift-negative-value -Wsign-compare -Wtype-limits -Wcast-qual -Wundef -Wbad-function-cast -Wold-style-definition -Wpacked -Wstrict-prototypes -Wstack-usage=2048")
set(COMPILE_WARING_FLAG " -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal -Wshadow -Wtrampolines -Wdate-time ")## -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal 
include(${HOME_PATH}/cmake/tool_chain/uniproton_tool_chain_gcc_arm64.cmake)
endif()

if (${CPU_TYPE} STREQUAL "x86_64" OR ${CPU_TYPE} STREQUAL "uvp")
#### 统一告警选项,请审慎增删
set(STRONG_COMPILE_WARING_FLAG "-Wunused -Wredundant-decls -Wfloat-conversion -Wwrite-strings -Wunused-macros -Wswitch-default -Wshift-overflow=2 -Wnested-externs -Wmissing-include-dirs -Wlogical-op -Wjump-misses-init -Wformat-security -Wvla -Wframe-larger-than=4096 -Wduplicated-cond  -Wdisabled-optimization -Wduplicated-branches -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wpointer-arith -Wshift-negative-value -Wsign-compare -Wtype-limits -Wcast-qual -Wundef -Wbad-function-cast -Wold-style-definition -Wpacked -Wstrict-prototypes -Wstack-usage=2048")
set(COMPILE_WARING_FLAG " -Wall -Wextra -Wformat=2 -Wfloat-equal -Wshadow -Wtrampolines -Wdate-time ")## -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal 
include(${HOME_PATH}/cmake/tool_chain/uniproton_tool_chain_gcc_x86_64.cmake)
endif()

if (${CPU_TYPE} STREQUAL "rv64virt" OR ${CPU_TYPE} STREQUAL "ds-d1s" )
#### 统一告警选项,请审慎增删
set(STRONG_COMPILE_WARING_FLAG "-Wunused -Wredundant-decls -Wfloat-conversion -Wwrite-strings -Wunused-macros -Wswitch-default -Wshift-overflow=2 -Wnested-externs -Wmissing-include-dirs -Wlogical-op -Wjump-misses-init -Wformat-security -Wvla -Wframe-larger-than=4096 -Wduplicated-cond  -Wdisabled-optimization -Wduplicated-branches -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wpointer-arith -Wshift-negative-value -Wsign-compare -Wtype-limits -Wcast-qual -Wundef -Wbad-function-cast -Wold-style-definition -Wpacked -Wstrict-prototypes -Wstack-usage=2048")
set(COMPILE_WARING_FLAG " -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal -Wshadow -Wtrampolines -Wdate-time ")## -Wall -Werror  -Wextra -Wformat=2 -Wfloat-equal 
include(${HOME_PATH}/cmake/tool_chain/uniproton_tool_chain_gcc_riscv64.cmake)
endif()

if (${COMPILE_OPTION} STREQUAL "coverity" OR ${COMPILE_OPTION} STREQUAL "fortify" )
    set(CMAKE_C_COMPILER_WORKS TRUE)
    set(CMAKE_CXX_COMPILER_WORKS TRUE)
    set(CMAKE_ASM_COMPILER_WORKS TRUE)
endif()

set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -r -D <TARGET> <OBJECTS>") 




cmake_minimum_required(VERSION 3.14.1) # cmake最低版本要求

project(lwip LANGUAGES C ASM) # 项目名字为lwip

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/x86_64-openeuler-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/x86_64-openeuler-linux-gnu-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/x86_64-openeuler-linux-gnu-as)

set(CC_OPTION "-g -O2 -mcmodel=large -static -nostdlib -nostdinc -fno-builtin -funwind-tables -nostartfiles -nodefaultlibs -mpreferred-stack-boundary=3 -mno-3dnow -mno-avx -mno-red-zone -Wl,--build-id=none -fno-builtin -fno-PIE -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-common -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -fno-builtin -fno-stack-protector -funsigned-char -fno-PIC")
set(CMAKE_C_FLAGS "${CC_OPTION}")

############################## 外部传入变量 ##############################
set(HOME_PATH ../../../../  )
set(INSTALL_DIR "$ENV{INSTALL_DIR}"  )

set(LWIP_SRC_PATH ${HOME_PATH}/src/net/lwip)

##############################include 头文件##############################
# 添加头文件
include_directories(
	# 安全函数头文件
	${HOME_PATH}/platform/libboundscheck/include

	# os头文件
	${HOME_PATH}/build/uniproton_config/config_x86_uvp
	${HOME_PATH}/demos/uvp/config/
	${HOME_PATH}/src/arch/include
	${HOME_PATH}/src/core/extend/lp/include
	${HOME_PATH}/src/core/ipc/include
	${HOME_PATH}/src/core/kernel/include
	${HOME_PATH}/src/include/uapi
	${HOME_PATH}/src/jsf/include
	${HOME_PATH}/src/mem/include
	${HOME_PATH}/src/om/include
	${HOME_PATH}/src/sandbox/include
	${HOME_PATH}/src/security/include
	${HOME_PATH}/src/utility/lib/include

	# lwip头文件
	${HOME_PATH}/demos/uvp/component/lwip
	${LWIP_SRC_PATH}/src/include
	${HOME_PATH}/src/net/lwip_port
	# ${LWIP_SRC_PATH}/src/include/compat/posix
	# ${LWIP_SRC_PATH}/src/include/compat/stdc

	${HOME_PATH}/src/libc/musl/include
	${HOME_PATH}/src/libc/litelibc/include
)

##############################添加要编译的.c文件##############################
file(GLOB_RECURSE LWIP_SRC  ${LWIP_SRC_PATH}/src/api/*.c 
							${LWIP_SRC_PATH}/src/core/*.c
							${LWIP_SRC_PATH}/src/core/ipv4/*.c
							${LWIP_SRC_PATH}/src/core/ipv6/*.c
							${LWIP_SRC_PATH}/src/netif/*.c
							${LWIP_SRC_PATH}/src/netif/ppp/*.c
							${LWIP_SRC_PATH}/src/netif/ppp/polarssl/*.c)

# 添加要编译的.c文件
set(LWIP_SRCS
	# lwip port
	${HOME_PATH}/src/net/adapter/src/sys_arch.c

	# lwip-2.1.3
	${LWIP_SRC}
)

##############################设置各个平台lwip库的名字############
set(LIB_LWIP_NAME "lwip")

#编译生成lwip.a库
add_library(${LIB_LWIP_NAME} STATIC ${LWIP_SRCS})

#安装lwip.a库
install(TARGETS ${LIB_LWIP_NAME} ARCHIVE DESTINATION ${INSTALL_DIR}/lib/uvp_x86)

#拷贝头文件到output目录
file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/arpa/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/arpa)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/net/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/net)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/posix/sys/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/posix/sys)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/compat/stdc/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/compat/stdc)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/apps/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/apps)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/priv/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/priv)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/lwip/prot/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/lwip/prot)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/ppp/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif/ppp)

file(GLOB include_files ${LWIP_SRC_PATH}/src/include/netif/ppp/polarssl/*.h)
install(FILES ${include_files} DESTINATION ${INSTALL_DIR}/include/netif/ppp/polarssl)
cmake_minimum_required(VERSION 3.12)


################ global parameter ###############################################
set(CUR_DIR 	${CMAKE_CURRENT_SOURCE_DIR}	)

################ parameter user should give !! ##################

set(HOME_DIR 		${UNIPROTON_ROOT_DIR} 	)
set(APPNAME		${APPNAME}		)

######### toolchain related ########

set(TOOLCHAINPATH	${TOOLCHAINPATH}	)
set(TOOLCHAINPREFIX	${TOOLCHAINPREFIX}	)
set(CC_FLAGS		"-static -march=rv64imafdc -mcmodel=medany -mabi=lp64d -Os -nostdlib -nostdinc -nostartfiles -nodefaultlibs -fno-builtin -fno-PIE -fno-PIC -fno-stack-protector -fzero-initialized-in-bss -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-aggressive-loop-optimizations -fno-inline-small-functions -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -finline-limit=20 -mstrict-align"
						)
set(AS_FLAGS		"-static -march=rv64imafdc -mcmodel=medany -mabi=lp64d -O0 -nostdlib -nostdinc -nostartfiles -nodefaultlibs -fno-builtin -fno-PIE -fno-PIC -fno-stack-protector -fzero-initialized-in-bss -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-aggressive-loop-optimizations -fno-inline-small-functions -fno-inline-small-functions -fno-inline-functions-called-once"		
						)
set(LD_FLAGS		"-Wl,--wrap=memset -Wl,--wrap=memcpy -T ${CUR_DIR}/build/milkvduo.ld"
						)


message("HOME_DIR = ${UNIPROTON_ROOT_DIR}")
message("APPNAME = ${APPNAME}")
message("TOOLCHAINPREFIX = ${TOOLCHAINPREFIX}")
message("CCFLAGS = ${CC_FLAGS}")
message("ASFLAGS = ${AS_FLAGS}")
message("LDFLAGS = ${LD_FLAGS}")

###############################################################

set(CMAKE_C_COMPILER 		${TOOLCHAINPREFIX}gcc	)
set(CMAKE_CXX_COMPILER		${TOOLCHAINPREFIX}g++	)
set(CMAK_ASM_COMPILER		${TOOLCHAINPREFIX}gcc	)
set(CMAKE_LINKER		${TOOLCHAINPREFIX}ld	)
set(CMAKE_C_FLAGS		"${CC_FLAGS}"		)
set(CMAKE_ASM_FLAGS		"${AS_FLAGS}"		)
set(CMAKE_LINK_FLAGS		"${LD_FLAGS}"		)
set(CMAKE_EXE_LINKER_FLAGS	"${LD_FLAGS}"		)
project(exmaples LANGUAGES C ASM)

include_directories(
	${CUR_DIR}/include
	${CUR_DIR}/include/hw/riscv64
	${CUR_DIR}/include/libc/include
	${CUR_DIR}/bsp/include
	${CUR_DIR}/config
	${HOME_DIR}/src/shell/adapter/include
    	${HOME_DIR}/src/shell/full/include
)


link_directories(
	${CUR_DIR}/libs
	${TOOLCHAINPATH}/lib/gcc/riscv64-unknown-elf/10.2.0/rv64imafdc/lp64d
)

link_libraries(
    -Wl,--start-group
    "${CUR_DIR}/libs/libCortexMXsec_c.lib"
    "${CUR_DIR}/libs/libMILKVDUOL.a"
    "${CUR_DIR}/libs/libmetal.a"
    "${CUR_DIR}/libs/libopen_amp.a"
    "${TOOLCHAINPATH}/lib/gcc/riscv64-unknown-elf/10.2.0/rv64imafdc/lp64d/libgcc.a"
    -Wl,--end-group
)


add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)
add_subdirectory(${HOME_DIR}/src/component/proxy proxy)
add_subdirectory(${HOME_DIR}/src/component/mica mica)
list(APPEND OBJS $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:app> $<TARGET_OBJECTS:mica>)
add_executable(${APPNAME} ${OBJS})

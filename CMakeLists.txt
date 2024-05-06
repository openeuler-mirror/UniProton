cmake_minimum_required(VERSION 3.14.1) #cmake最低版本要求

project(UniProton LANGUAGES C ASM) #项目名字为UniProton


############################## 外部传入变量 ##############################
set(CPU_TYPE "$ENV{CPU_TYPE}" )
set(PLAM_TYPE "$ENV{PLAM_TYPE}" )
set(LIB_TYPE "$ENV{LIB_TYPE}"  )
set(UNIPROTON_PACKING_PATH "$ENV{UNIPROTON_PACKING_PATH}" ) #UNIPROTON_PACKING_PATH
set(COMPILE_OPTION "$ENV{COMPILE_OPTION}" ) #编译选项 空为默认全编译 还有coverity/fortify
set(CONFIG_FILE_PATH "$ENV{CONFIG_FILE_PATH}" ) #defconfig及其头文件所在目录
string(TOUPPER ${PLAM_TYPE} PLAM_TYPE_UP) #转大写
set(SYSTEM "$ENV{SYSTEM}" )
set(CORE "$ENV{CORE}" )
set(LIB_RUN_TYPE "$ENV{LIB_RUN_TYPE}" )
set(HOME_PATH "$ENV{HOME_PATH}" )
set(BUILD_MACHINE_PLATFORM "$ENV{BUILD_MACHINE_PLATFORM}" )
set(RPROTON_BINARY_DIR "$ENV{RPROTON_BINARY_DIR}" )
#用于保存所有的对象库
set(ALL_OBJECT_LIBRARYS "" CACHE STRING INTERNAL)

if("${RPROTON_BINARY_DIR}" STREQUAL "")
  # this branch will be taken
  set(RPROTON_BINARY_DIR ${PROJECT_SOURCE_DIR}/build/output)
else()
  set(RPROTON_BINARY_DIR ${RPROTON_BINARY_DIR}/UniProton/tmp/output)
endif()

message("UniProton BINARY_DIR=${RPROTON_BINARY_DIR}")

######该参数的判断逻辑脚本来实现一个XCache的开关######
if (CACHE)
    find_program(CACHE_FOUND ${CACHE})
    set(CACHE_TOOL "${CACHE}")
endif()
if(CACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CACHE_TOOL})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CACHE_TOOL})
endif()

############################## 公共函数定义导入 ##############################
include(${HOME_PATH}/cmake/functions/uniproton_functions.cmake)

############################## 编译参数 ##################################
#下述这些参数的设置要放在项目设置之后！
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${RPROTON_BINARY_DIR}/${CPU_TYPE}/${PLAM_TYPE}/${LIB_RUN_TYPE}") #库文件输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

############################## 调试打印 ##############################
message("CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message("CMAKE_AR is ${CMAKE_AR}")
message("CMAKE_LINKER is ${CMAKE_LINKER}")
message("CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")
message("BUILD_MACHINE_PLATFORM= ${BUILD_MACHINE_PLATFORM}")
message("CONFIG_FILE_PATH= ${CONFIG_FILE_PATH}")

if (${COMPILE_MODE} STREQUAL "debug")
add_compile_options("-g") #编译 -g 选项，打开后可以在sdk调试时看到调试信息
endif()
message("=============== COMPILE_MODE is ${COMPILE_MODE} ===============")
############################## 通过.config文件引入编译宏 ##################
# #函数import_kconfig把.config文件中的定义转换为cmake变量，用于后续子文件夹中决定是否编译某库
import_kconfig(${CONFIG_FILE_PATH}/defconfig)
##############################include 头文件##############################
#可能和平台相关
include_directories(
	${CONFIG_FILE_PATH}
	./src/arch/include
	./src/core/ipc/include
	./src/core/kernel/include
	./src/core/extend/include
	./src/include/uapi
	./src/mem/include
	./src/om/include
	./src/utility/lib/include
	./platform/libboundscheck/include
)

if (${CONFIG_OS_OPTION_POSIX_SIGNAL})
    if (NOT CONFIG_OS_OPTION_POSIX)
        message(FATAL_ERROR "if config CONFIG_OS_OPTION_POSIX_SIGNAL must config CONFIG_OS_OPTION_POSIX")
    endif()
endif()

if (${CONFIG_OS_OPTION_POSIX})
set(muslsrcdir ${CMAKE_CURRENT_SOURCE_DIR}/src/libc/musl)
execute_process(
    WORKING_DIRECTORY ${muslsrcdir}
    COMMAND sh -c "
        echo ' Build generic_include_file!'
        mkdir -p include/bits/
        cp arch/generic/bits/*.h include/bits/
        cp arch/${CMAKE_SYSTEM_PROCESSOR}/bits/*.h include/bits/
        sed -f tools/mkalltypes.sed arch/${CMAKE_SYSTEM_PROCESSOR}/bits/alltypes.h.in include/alltypes.h.in > include/bits/alltypes.h
        cp arch/${CMAKE_SYSTEM_PROCESSOR}/bits/syscall.h.in include/bits/syscall.h
        sed -n -e s/__NR_/SYS_/p < arch/${CMAKE_SYSTEM_PROCESSOR}/bits/syscall.h.in >> include/bits/syscall.h
        echo ' End generic_include_file!'
    "
)

include_directories(
        ./src/libc/litelibc/internal/include
        ./src/libc/litelibc/include
        ./src/libc/musl/src/include
        ./src/libc/musl/include
        ./src/core/ipc/rwlock
        ./src/kal/include
        ./src/drivers/i210
        ./src/core/kernel/task
)

if (${CONFIG_OS_OPTION_LINUX})
set(LINUX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/osal/linux)
include_directories(
    ${LINUX_INCLUDE_DIR}/include_self_def
)
endif()

if (${CONFIG_OS_SUPPORT_I40E})
include_directories(
    ./src/drivers/i40e
)
endif()

if (${PLAM_TYPE} STREQUAL "x86_64")
    add_compile_options(-std=gnu99)
else()
    add_compile_options(-std=c99)
endif()

add_compile_options(
        -ffreestanding 
        -nostdinc 
        -Wno-undef 
        -Wno-strict-prototypes 
        -Wno-redundant-decls
        -Wno-cast-qual
        -Wno-parentheses
        -Wno-pointer-sign
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-jump-misses-init
        -Wno-old-style-definition
        -Wno-shadow
        -Wno-unused-macros
        )
endif()

if (${CONFIG_OS_OPTION_OPENAMP})
include_directories(
	./thirdpart/openamp
)
endif()

if (${CONFIG_OS_OPTION_NUTTX_VFS})
include_directories(
  ./src/fs/include
)
endif()

if (NOT ${COMPILE_OPTION} STREQUAL "UniProton")
###添加安全库的cmakelists
add_subdirectory(platform)
endif()

if (NOT ${COMPILE_OPTION} STREQUAL "sec")
###添加源码目录的cmakelists
add_subdirectory(src)

##############################不同平台，根据需要链接出不同的lib库############
include(./cmake/tool_chain/${CPU_TYPE}_${PLAM_TYPE}.cmake)
endif()

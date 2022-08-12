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
	./src/include/uapi
	./src/mem/include
	./src/om/include
	./src/utility/lib/include
	./platform/libboundscheck/include
)


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

set(OBJCOPY_PATH "$ENV{OBJCOPY_PATH}" )
##最终的链接目标，会生成对应的.a库
#################################################
## arch代码
#################################################

#将所有对象库添加到列表中
foreach(FILE_NAME ${ALL_OBJECT_LIBRARYS})
    list(FIND exclude "${FILE_NAME}" is_include)
    if(${is_include} EQUAL -1)
    list(APPEND UVP_X86_SRCS
        $<TARGET_OBJECTS:${FILE_NAME}>
    )
    endif()
endforeach()

#编译结果
string(TOUPPER ${PLAM_TYPE} PLAM_TYPE_UP)
string(TOUPPER ${CPU_TYPE} CPU_TYPE_UP)
#编译.a库
add_library(uvp_x86 STATIC "${UVP_X86_SRCS}")
add_custom_target(cleanobj)
add_custom_command(TARGET cleanobj POST_BUILD
                   COMMAND echo "Finish Building!"
                   )

if (${COMPILE_MODE} STREQUAL "debug")
    message("=============== COMPILE_MODE is ${COMPILE_MODE} ===============")
endif()
 ####以下为uvp_x86 make install打包脚本#####
set(uvp_x86_export modules)

# 下面的变量分别定义了安装根目录、头文件安装目录、动态库安装目录、静态库安装目录、OBJECT文件安装目录、可执行程序安装目录、配置文件安装目录。
# 注意：所有安装路径必须是相对CMAKE_INSTALL_PREFIX的相对路径，不可以使用绝对路径!!!
# 否则安装目录下的配置文件(foo-config.cmake, foo-tragets.cmake等)拷贝到其它目录时无法工作。
set(INSTALL_UVP_X86_BASE_DIR               .)
set(INSTALL_UVP_X86_INCLUDE_DIR            UniProton/include)
set(INSTALL_UVP_X86_INCLUDE_SEC_DIR        libboundscheck/include)
set(INSTALL_UVP_X86_ARCHIVE_DIR            UniProton/lib/uvp)
set(INSTALL_UVP_X86_ARCHIVE_SEC_DIR        libboundscheck/lib/uvp)
set(INSTALL_UVP_X86_ARCHIVE_LINUX_DIR      linux/lib/uvp)
set(INSTALL_UVP_X86_ARCHIVE_CONFIG_DIR     UniProton/config)
set(INSTALL_UVP_X86_CONFIG_DIR             cmake/uvp)



include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/tool_chain/uvp_x86_config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/UniProton-uvp_x86-config.cmake
    INSTALL_DESTINATION ${INSTALL_UVP_X86_CONFIG_DIR}
    PATH_VARS
    INSTALL_UVP_X86_BASE_DIR
    INSTALL_UVP_X86_INCLUDE_DIR
    INSTALL_UVP_X86_INCLUDE_SEC_DIR
    INSTALL_UVP_X86_ARCHIVE_DIR
    INSTALL_UVP_X86_ARCHIVE_SEC_DIR
    INSTALL_UVP_X86_ARCHIVE_CONFIG_DIR
    INSTALL_UVP_X86_CONFIG_DIR
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
)
install(EXPORT ${uvp_x86_export}
        NAMESPACE UniProton::
        FILE UniProton-uvp_x86-targets.cmake
        DESTINATION ${INSTALL_UVP_X86_CONFIG_DIR}
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UniProton-uvp_x86-config.cmake
    DESTINATION ${INSTALL_UVP_X86_CONFIG_DIR}
)

install(TARGETS
    uvp_x86
    EXPORT ${uvp_x86_export}
    ARCHIVE DESTINATION ${INSTALL_UVP_X86_ARCHIVE_DIR}/
)

if (${COMPILE_OPTION} STREQUAL "coverity" OR ${COMPILE_OPTION} STREQUAL "fortify" OR ${COMPILE_OPTION} STREQUAL "UniProton")
    message("Don't Install Sec Lib In ${COMPILE_OPTION}")
else()
    install(TARGETS
        CortexMXsec_c
        EXPORT ${uvp_x86_export}
        ARCHIVE DESTINATION ${INSTALL_UVP_X86_ARCHIVE_SEC_DIR}
    )

    if (NOT "${RPROTON_INSTALL_FILE_OPTION}" STREQUAL "SUPER_BUILD")
        ##{GLOB 所有文件 | GLOB_RECURSE 递归查找文件&文件夹}
        file(GLOB glob_sec_files  ${PROJECT_SOURCE_DIR}/platform/libboundscheck/include/*.h)
        install(FILES
            ${glob_sec_files}
            DESTINATION ${INSTALL_UVP_X86_INCLUDE_SEC_DIR}
        )
    endif()
endif()

install(FILES
    ${PROJECT_SOURCE_DIR}/build/uniproton_config/config_x86_uvp/prt_buildef.h
    DESTINATION ${INSTALL_UVP_X86_ARCHIVE_CONFIG_DIR}/uvp_x86/config_x86_uvp
)
if (NOT "${RPROTON_INSTALL_FILE_OPTION}" STREQUAL "SUPER_BUILD")
    install(FILES
        ${PROJECT_SOURCE_DIR}/src/config/prt_config.c
        ${PROJECT_SOURCE_DIR}/src/config/prt_config_internal.h
        ${PROJECT_SOURCE_DIR}/src/config/config/prt_config.h
        DESTINATION ${INSTALL_UVP_X86_ARCHIVE_CONFIG_DIR}
    )

    ##{GLOB 所有文件 | GLOB_RECURSE 递归查找文件&文件夹}
    file(GLOB hw_drv_include_files  ${PROJECT_SOURCE_DIR}/src/include/uapi/hw/uvp_x86/*)
    install(FILES
        ${hw_drv_include_files}
        DESTINATION ${INSTALL_UVP_X86_INCLUDE_DIR}/hw/uvp_x86
    )

    install(FILES
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_clk.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_cpup.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_err.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_errno.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_event.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_exc.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_hook.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_hwi.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_idle.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_mem.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_module.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_queue.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_sem.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_sys.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_task.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_tick.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_timer.h
        ${PROJECT_SOURCE_DIR}/src/include/uapi/prt_typedef.h
        DESTINATION ${INSTALL_UVP_X86_INCLUDE_DIR}/
    )
endif()

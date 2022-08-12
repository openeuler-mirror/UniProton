set(BUILD_DIR "$ENV{BUILD_TMP_DIR}" ) #version id
set(OBJCOPY_PATH "$ENV{OBJCOPY_PATH}" ) #OBJCOPY_PATH



##最终的链接目标，会生成对应的.a库
#################################################
## arch代码
#################################################
list(APPEND ARCH_SRCS
    $<TARGET_OBJECTS:prt_hw_boot>
    $<TARGET_OBJECTS:prt_port>
    $<TARGET_OBJECTS:prt_dispatch>
    $<TARGET_OBJECTS:prt_hwi>
    $<TARGET_OBJECTS:prt_exc>
    $<TARGET_OBJECTS:prt_hw_exc>
    $<TARGET_OBJECTS:prt_vector>
    $<TARGET_OBJECTS:prt_vi_dispatch>
    $<TARGET_OBJECTS:prt_hw_tick>
    $<TARGET_OBJECTS:prt_hw_tick_minor>
    $<TARGET_OBJECTS:prt_hw>
    $<TARGET_OBJECTS:prt_div64>
)

#################################################
## kernel代码
#################################################
list(APPEND KERNEL_SRCS
    $<TARGET_OBJECTS:prt_sem>
    $<TARGET_OBJECTS:prt_sem_init>
    $<TARGET_OBJECTS:prt_sem_minor>
    $<TARGET_OBJECTS:prt_irq>
    $<TARGET_OBJECTS:prt_sys>
    $<TARGET_OBJECTS:prt_sys_init>
    $<TARGET_OBJECTS:prt_sys_time>
    $<TARGET_OBJECTS:prt_amp_task>
    $<TARGET_OBJECTS:prt_amp_task_del>
    $<TARGET_OBJECTS:prt_amp_task_init>
    $<TARGET_OBJECTS:prt_amp_task_minor>
    $<TARGET_OBJECTS:prt_task>
    $<TARGET_OBJECTS:prt_taskself_id>
    $<TARGET_OBJECTS:prt_task_attrib>
    $<TARGET_OBJECTS:prt_task_del>
    $<TARGET_OBJECTS:prt_task_global>
    $<TARGET_OBJECTS:prt_task_info>
    $<TARGET_OBJECTS:prt_task_init>
    $<TARGET_OBJECTS:prt_task_minor>
    $<TARGET_OBJECTS:prt_task_priority>
    $<TARGET_OBJECTS:prt_task_sem>
    $<TARGET_OBJECTS:prt_tick>
    $<TARGET_OBJECTS:prt_tick_init>
    $<TARGET_OBJECTS:prt_timer>
    $<TARGET_OBJECTS:prt_swtmr>
    $<TARGET_OBJECTS:prt_swtmr_init>
    $<TARGET_OBJECTS:prt_swtmr_minor>
    $<TARGET_OBJECTS:prt_kexc>
    $<TARGET_OBJECTS:prt_exc>
    $<TARGET_OBJECTS:prt_timer_minor>
)
list(APPEND IPC_SRCS
    $<TARGET_OBJECTS:prt_event>
    $<TARGET_OBJECTS:prt_queue>
    $<TARGET_OBJECTS:prt_queue_del>
    $<TARGET_OBJECTS:prt_queue_minor>
    $<TARGET_OBJECTS:prt_queue_init>
)
list(APPEND MEM_SRCS
    $<TARGET_OBJECTS:prt_fscmem>
    $<TARGET_OBJECTS:prt_mem>
)
list(APPEND OM_SRCS
    $<TARGET_OBJECTS:prt_cpup>
    $<TARGET_OBJECTS:prt_cpup_minor>
    $<TARGET_OBJECTS:prt_cpup_thread>
    $<TARGET_OBJECTS:prt_cpup_thread_64>
    $<TARGET_OBJECTS:prt_cpup_thread_init>
    $<TARGET_OBJECTS:prt_cpup_warn>
    $<TARGET_OBJECTS:prt_err>
    $<TARGET_OBJECTS:prt_err_init>
    $<TARGET_OBJECTS:prt_hook_init>
    $<TARGET_OBJECTS:prt_rnd_set>
)
list(APPEND BASE_LIB_SRCS
    $<TARGET_OBJECTS:prt_lib_math64>
    $<TARGET_OBJECTS:prt_lib_version>
)



#编译结果
string(TOUPPER ${PLAM_TYPE} PLAM_TYPE_UP)
string(TOUPPER ${CPU_TYPE} CPU_TYPE_UP)
#编译通用的.a库
add_library(CortexMXarch  STATIC "${ARCH_SRCS}")
add_library(CortexMXkernel  STATIC "${KERNEL_SRCS}" "${BASE_LIB_SRCS}")
add_library(CortexMXmem  STATIC "${MEM_SRCS}")
add_library(CortexMXom  STATIC "${OM_SRCS}")
add_library(CortexMXipc  STATIC "${IPC_SRCS}")

set_target_properties(CortexMXarch PROPERTIES SUFFIX ".lib")
set_target_properties(CortexMXkernel PROPERTIES SUFFIX ".lib")
set_target_properties(CortexMXmem PROPERTIES SUFFIX ".lib")
set_target_properties(CortexMXom PROPERTIES SUFFIX ".lib")
set_target_properties(CortexMXipc PROPERTIES SUFFIX ".lib")

add_custom_target(cleanobj)
add_custom_command(TARGET cleanobj POST_BUILD
                   COMMAND echo "Finish Building!"
                   )

if (${COMPILE_MODE} STREQUAL "debug")
    message("=============== COMPILE_MODE is ${COMPILE_MODE} ===============")
else()
    add_custom_command(
        TARGET CortexMXarch
        POST_BUILD
        COMMAND sh ${PROJECT_SOURCE_DIR}/cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh ${OBJCOPY_PATH} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} "CortexMXarch.lib"
    )

    add_custom_command(
        TARGET CortexMXkernel
        POST_BUILD
        COMMAND sh ${PROJECT_SOURCE_DIR}/cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh ${OBJCOPY_PATH} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} "CortexMXkernel.lib"
    )

    add_custom_command(
        TARGET CortexMXmem
        POST_BUILD
        COMMAND sh ${PROJECT_SOURCE_DIR}/cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh ${OBJCOPY_PATH} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} "CortexMXmem.lib"
    )

    add_custom_command(
        TARGET CortexMXom
        POST_BUILD
        COMMAND sh ${PROJECT_SOURCE_DIR}/cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh ${OBJCOPY_PATH} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} "CortexMXom.lib"
    )

    add_custom_command(
        TARGET CortexMXipc
        POST_BUILD
        COMMAND sh ${PROJECT_SOURCE_DIR}/cmake/common/build_auxiliary_script/make_lib_rename_file_type.sh ${OBJCOPY_PATH} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} "CortexMXipc.lib"
    )
endif()
####以下为m4 make install打包脚本#####
set(m4_cortex_export modules)

# 下面的变量分别定义了安装根目录、头文件安装目录、动态库安装目录、静态库安装目录、OBJECT文件安装目录、可执行程序安装目录、配置文件安装目录。
# 注意：所有安装路径必须是相对CMAKE_INSTALL_PREFIX的相对路径，不可以使用绝对路径!!!
# 否则安装目录下的配置文件(foo-config.cmake, foo-tragets.cmake等)拷贝到其它目录时无法工作。
set(INSTALL_M4_CORTEX_BASE_DIR               .)
set(INSTALL_M4_CORTEX_INCLUDE_DIR            UniProton/include)
set(INSTALL_M4_CORTEX_INCLUDE_SEC_DIR        libboundscheck/include)
set(INSTALL_M4_CORTEX_ARCHIVE_DIR            UniProton/lib/cortex_m4)
set(INSTALL_M4_CORTEX_ARCHIVE_SEC_DIR        libboundscheck/lib/cortex_m4)
set(INSTALL_M4_CORTEX_ARCHIVE_CONFIG_DIR     UniProton/config)
set(INSTALL_M4_CORTEX_CONFIG_DIR             cmake/cortex_m4)




include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/tool_chain/m4-cortex-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/UniProton-m4-cortex-config.cmake
    INSTALL_DESTINATION ${INSTALL_M4_CORTEX_CONFIG_DIR}
    PATH_VARS
    INSTALL_M4_CORTEX_BASE_DIR
    INSTALL_M4_CORTEX_INCLUDE_DIR
    INSTALL_M4_CORTEX_INCLUDE_SEC_DIR
    INSTALL_M4_CORTEX_ARCHIVE_DIR
    INSTALL_M4_CORTEX_ARCHIVE_SEC_DIR
    INSTALL_M4_CORTEX_ARCHIVE_CONFIG_DIR
    INSTALL_M4_CORTEX_CONFIG_DIR
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
)
install(EXPORT ${m4_cortex_export}
        NAMESPACE UniProton::
        FILE UniProton-m4-cortex-targets.cmake
        DESTINATION ${INSTALL_M4_CORTEX_CONFIG_DIR}
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/UniProton-m4-cortex-config.cmake
    DESTINATION ${INSTALL_M4_CORTEX_CONFIG_DIR}
)

install(TARGETS
    CortexMXarch
    CortexMXkernel
    CortexMXmem
    CortexMXom
    CortexMXipc
    EXPORT ${m4_cortex_export}
    ARCHIVE DESTINATION ${INSTALL_M4_CORTEX_ARCHIVE_DIR}/
)
if (${COMPILE_OPTION} STREQUAL "coverity" OR ${COMPILE_OPTION} STREQUAL "fortify" OR ${COMPILE_OPTION} STREQUAL "UniProton")
    message("Don't Install Sec Lib In ${COMPILE_OPTION}")
else()
    install(TARGETS
        CortexMXsec_c
        EXPORT ${m4_cortex_export}
        ARCHIVE DESTINATION ${INSTALL_M4_CORTEX_ARCHIVE_SEC_DIR}
    )

    if (NOT "${RPROTON_INSTALL_FILE_OPTION}" STREQUAL "SUPER_BUILD")
        ##{GLOB 所有文件 | GLOB_RECURSE 递归查找文件&文件夹}
        file(GLOB glob_sec_files  ${PROJECT_SOURCE_DIR}/platform/libboundscheck/include/*.h)
        install(FILES
            ${glob_sec_files}
            DESTINATION ${INSTALL_M4_CORTEX_INCLUDE_SEC_DIR}
        )
    endif()
endif()
install(FILES
    ${PROJECT_SOURCE_DIR}/build/uniproton_config/config_m4/prt_buildef.h
    DESTINATION ${INSTALL_M4_CORTEX_ARCHIVE_CONFIG_DIR}/cortex_m4/config_m4
)
if (NOT "${RPROTON_INSTALL_FILE_OPTION}" STREQUAL "SUPER_BUILD")
    ##{GLOB 所有文件 | GLOB_RECURSE 递归查找文件&文件夹}

    install(FILES
        ${PROJECT_SOURCE_DIR}/src/config/prt_config.c
        ${PROJECT_SOURCE_DIR}/src/config/prt_config_internal.h
        ${PROJECT_SOURCE_DIR}/src/config/config/prt_config.h
        DESTINATION ${INSTALL_M4_CORTEX_ARCHIVE_CONFIG_DIR}
    )

    ##{GLOB 所有文件 | GLOB_RECURSE 递归查找文件&文件夹}
    file(GLOB hw_drv_include_files  ${PROJECT_SOURCE_DIR}/src/include/uapi/hw/armv7-m/*)
    install(FILES
        ${hw_drv_include_files}
        DESTINATION ${INSTALL_M4_CORTEX_INCLUDE_DIR}/hw/armv7-m
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
        DESTINATION ${INSTALL_M4_CORTEX_INCLUDE_DIR}/
    )
endif()

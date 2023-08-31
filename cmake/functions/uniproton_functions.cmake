# Date:   2019-11-11
# Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
###########################################################################


############################## 公共函数定义区域 ##############################
## 1. import defconfig
##把.config文件转换为cmake命名空间的变量，比如把CONFIG_OS_ARCH_ARM32VX=y 转化为 cmake变量
function(import_kconfig config_file)##.config文件转换为cmake命名空间的变量
    #### 读取Config文件
    file(STRINGS ${config_file} config_list REGEX "^CONFIG_" ENCODING "UTF-8")#### 读取Config文件

    ##处理各个匹配行
    foreach (config ${config_list})##处理各个匹配行
      ##获取变量值
      ##字符串匹配‘=’号
      string(REGEX MATCH "=(.+$)" conf_value ${config})##字符串匹配‘=’号
      ## 获取匹配值
      set(conf_value ${CMAKE_MATCH_1})##获取匹配值
      ##匹配CONFIG_OS_HARDWARE_PLATFORM="OS_ARM7" 这种有""的情形
      if("${conf_value}" MATCHES "^\"(.*)\"$") 
        ##设置环境变量值
        set(conf_value ${CMAKE_MATCH_1})##设置环境变量值
      ##定义结束
      endif()##定义结束
      ##获取变量名
      ##字符串匹配‘=’号
      string(REGEX MATCH "[^=]+" conf_name ${config})##字符串匹配‘=’号
      ##声明cmake全局变量
      ##设置环境变量值
      set("${conf_name}" "${conf_value}" PARENT_SCOPE)##设置环境变量值
      ##message("${conf_name}=${conf_value}")
    ##遍历结束符
    endforeach()##遍历结束符
##功能结束符
endfunction()##功能结束符






function(cc_object target sources cppincs  cflags component_name working_directory)
    # @target Object: the name of the target
    # @sources List[str]: the code source path of list
    # @cppincs List[str]: the include directories path of list
    # @cppdefines List[str]: the defines of list
    # @cflags List[str]: the c compile flags of list
    # @component_name str: the component name of the atrget
    # @working_directory str: the directory of command execute
    list(JOIN ${cppincs} "\t-I" target_incflags)
    set(obj_install_dir ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${component_name})
    file(MAKE_DIRECTORY ${obj_install_dir})
    
    
    
    foreach(source_path IN LISTS ${sources})
        get_filename_component(source_name ${source_path} NAME)
        # .c替换为.eln
        string(REGEX REPLACE \\.c \.o target_name ${source_name})
        string(REPLACE ${working_directory}/ "" new_source_path ${source_path})
        string(REGEX REPLACE .*/ "" current_directory ${working_directory})
        add_custom_command(
            OUTPUT ${obj_install_dir}/${target_name}
            COMMAND ${cc} ${${cflags}} -I${target_incflags}  -o ${obj_install_dir}/${target_name} -c ${new_source_path}
            DEPENDS ${source_path}
            WORKING_DIRECTORY ${working_directory}
        )
        add_custom_target(
            ${target_name}_custom
            ALL
            DEPENDS ${obj_install_dir}/${target_name}
        )
        
        list(APPEND ${target}_list_dep ${target_name}_custom )
        list(APPEND ${target}_list__ ${obj_install_dir}/${target_name} )
    endforeach()
    set(${target}_PATH_LIST ${${target}_list__} PARENT_SCOPE)
    set(${target}_NAME_LIST ${${target}_list_dep} PARENT_SCOPE)
endfunction()


function(asm_object target sources   cflags component_name working_directory)
    # @target Object: the name of the target
    # @sources List[str]: the code source path of list
    # @cppincs List[str]: the include directories path of list
    # @cppdefines List[str]: the defines of list
    # @cflags List[str]: the c compile flags of list
    # @component_name str: the component name of the atrget
    # @working_directory str: the directory of command execute
    set(obj_install_dir ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${component_name})
    file(MAKE_DIRECTORY ${obj_install_dir})
    
    
    
    foreach(source_path IN LISTS ${sources})
        get_filename_component(source_name ${source_path} NAME)
        string(REGEX REPLACE \\.asm \.eln target_name ${source_name})
        string(REPLACE ${working_directory}/ "" new_source_path ${source_path})
        string(REGEX REPLACE .*/ "" current_directory ${working_directory})
        add_custom_command(
            OUTPUT ${obj_install_dir}/${target_name}
            COMMAND ${asm} ${${cflags}}   -o ${obj_install_dir}/${target_name} -c ${new_source_path}
            DEPENDS ${source_path}
            WORKING_DIRECTORY ${working_directory}
        )
        add_custom_target(
            ${target_name}_custom
            ALL
            DEPENDS ${obj_install_dir}/${target_name}
        )
        
        list(APPEND ${target}_list_dep ${target_name}_custom )
        list(APPEND ${target}_list__ ${obj_install_dir}/${target_name} )
    endforeach()
    set(${target}_PATH_LIST ${${target}_list__} PARENT_SCOPE)
    set(${target}_NAME_LIST ${${target}_list_dep} PARENT_SCOPE)
endfunction()




    
function(ar_library target_name target_suffix target_path  ar_flag object_name object_path)
    # @target Object: the name of the target
    # @sources List[str]: the code source path of list
    # @cppincs List[str]: the include directories path of list
    # @cppdefines List[str]: the defines of list
    # @cflags List[str]: the c compile flags of list
    # @component_name str: the component name of the atrget
    # @working_directory str: the directory of command execute

    add_custom_command(
        OUTPUT ${target_path}/${target_name}.${target_suffix}
        COMMAND ${ar} ${ar_flag} ${target_path}/${target_name}.${target_suffix} ${object_path}
        WORKING_DIRECTORY ${target_path}
    )
    add_custom_target(
        ${target_name}
        ALL
        DEPENDS ${target_path}/${target_name}.${target_suffix}
    )
    foreach(object_target  ${object_name})
        add_dependencies(${target_name} ${object_target})
    endforeach()
endfunction()


function(link_library target_name  target_path  link_flag object_name object_path)
    # @target Object: the name of the target
    # @sources List[str]: the code source path of list
    # @cppincs List[str]: the include directories path of list
    # @cppdefines List[str]: the defines of list
    # @cflags List[str]: the c compile flags of list
    # @component_name str: the component name of the atrget
    # @working_directory str: the directory of command execute

    add_custom_command(
        OUTPUT ${target_path}/${target_name}
        COMMAND ${link} ${${link_flag}} -o ${target_path}/${target_name} ${object_path}
        WORKING_DIRECTORY ${target_path}
    )
    add_custom_target(
        ${target_name}
        ALL
        DEPENDS ${target_path}/${target_name}
    )
    foreach(object_target  ${object_name})
        add_dependencies(${target_name} ${object_target})
    endforeach()
endfunction()

function(add_library_ex file_name)
    #[[
    函数功能：传入要生成的对象库的文件名（不包含路径），生成对应的对象库并添加到全局变量中
    用于后续能自动生成打包文件，无需开发人员再次人工指定
    局限性：
    1、只能传入单个文件，确保每个对象库只包含单个文件
    2、生成对象库的文件，必然都是.c 或者.S文件
    ]]
    #查询传递进来的文件名是否包含路径
    string(FIND ${file_name} "/" idx REVERSE)
    if(${idx} GREATER -1)
        string(SUBSTRING ${file_name} ${idx} -1 str2)
        string(REPLACE "/" "" object_name ${str2})
        string(REPLACE ".c" "" object_name ${object_name})
    else()
        #将.c .S文件去除后缀名，只保留文件名（文件名作为对象库的名字）
        string(REPLACE ".c" "" object_name ${file_name})
    endif()
 
    string(REPLACE ".S" "" object_name ${object_name})
    add_library(${object_name} OBJECT ${file_name})
    #将对象库添加到全局变量中
    list(APPEND ALL_OBJECT_LIBRARYS ${object_name})
    set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
    
endfunction()


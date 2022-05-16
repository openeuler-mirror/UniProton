set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -r  <TARGET> <OBJECTS>")

if (${COMPILE_OPTION} STREQUAL "coverity" OR ${COMPILE_OPTION} STREQUAL "fortify" OR ${COMPILE_OPTION} STREQUAL "single")
    message("Don't Compile Sec Lib In ${COMPILE_OPTION}")
else()
    include_directories(libboundscheck/include)
    include_directories(libboundscheck/src)

    set(LIB_SEC_NAME "CortexMXsec_c")
    
    file(GLOB_RECURSE SEC_SRC RELATIVE ${CMAKE_CURRENT_LIST_DIR} "./libboundscheck/src/*.c") #遍历搜索src目录下的所有c文件,RELATIVE为使用相对路径
    add_library(${LIB_SEC_NAME} STATIC ${SEC_SRC}) #在文件夹上层选择性编译与否

    set_target_properties(${LIB_SEC_NAME} PROPERTIES SUFFIX ".lib")

endif()
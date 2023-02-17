add_library_ex(prt_cpup.c)
add_library_ex(prt_cpup_minor.c)

##条件判断
if(${CONFIG_INTERNAL_OS_CPUP_THREAD})##条件判断
##根据条件添加库
add_library_ex(prt_cpup_thread.c)##根据条件添加库
##条件结束符号
endif()##条件结束符号

##条件判断
if(${CONFIG_INTERNAL_OS_CPUP_THREAD})##条件判断
##根据条件添加库
add_library_ex(prt_cpup_thread_init.c)##根据条件添加库
##条件结束符号
endif()##条件结束符号

##条件判断
if(${CONFIG_INTERNAL_OS_CPUP_THREAD})##条件判断
##根据条件添加库
add_library_ex(prt_cpup_thread_64.c)##根据条件添加库
##条件结束符号
endif()##条件结束符号

##条件判断
if(${CONFIG_OS_OPTION_CPUP_WARN})##条件判断
##根据条件添加库
add_library_ex(prt_cpup_warn.c)##根据条件添加库
##条件结束符号
endif()##条件结束符号

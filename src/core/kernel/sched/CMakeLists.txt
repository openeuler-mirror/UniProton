#条件判断
if(${CONFIG_OS_OPTION_SMP})##条件判断
    ##根据条件添加库
    add_library_ex(prt_rt_single.c)
    ##条件结束符号
endif()##条件结束符号

if(${CONFIG_OS_OPTION_SMP})##条件判断
    ##根据条件添加库
    add_library_ex(prt_sched.c)
    add_library_ex(prt_sched_class.c)
    add_library_ex(prt_sched_init.c)
    ##条件结束符号
else()
    add_library_ex(prt_sched_single.c)
endif()##条件结束符号



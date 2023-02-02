
##条件判断
##if(${CONFIG_INTERNAL_OS_SCHEDULE_SINGLE_CORE_BY_CCODE})##条件判断
    ##根据条件添加库
    add_library_ex(prt_sched_single.c)
    ##条件结束符号
##endif()##条件结束符号



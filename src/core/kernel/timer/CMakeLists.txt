add_library(prt_timer OBJECT prt_timer.c)
add_library(prt_timer_minor OBJECT prt_timer_minor.c)

if(${CONFIG_INTERNAL_OS_SWTMR})
    add_subdirectory(swtmr)
endif()


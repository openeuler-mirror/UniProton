add_library_ex(prt_timer.c)
add_library_ex(prt_timer_minor.c)

if(${CONFIG_INTERNAL_OS_SWTMR})
    add_subdirectory(swtmr)
endif()


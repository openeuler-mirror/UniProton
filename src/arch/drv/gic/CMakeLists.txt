if("${CONFIG_INTERNAL_OS_GIC_VER}" STREQUAL "gicv600")
    include_directories(./gicv600)
endif()

add_library_ex(prt_gic_init.c)
add_library_ex(prt_gicd_init.c)
add_library_ex(prt_gicr_init.c)
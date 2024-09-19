add_library(fixme OBJECT fixme.c)
target_include_directories(fixme PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)
add_library(lwip_ifaddrs OBJECT lwip_ifaddrs.c)
target_include_directories(lwip_ifaddrs PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)

list(APPEND ALL_OBJECT_LIBRARYS fixme lwip_ifaddrs)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
add_library(driverif OBJECT driverif.c)
target_include_directories(driverif PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)
add_library(netdb OBJECT netdb.c)
target_include_directories(netdb PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)
add_library(sockets OBJECT sockets.c)
target_include_directories(sockets PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)
target_compile_options(sockets PUBLIC 
    -Wno-unused-function
)
add_library(sys_arch OBJECT sys_arch.c)
target_include_directories(sys_arch PUBLIC 
    ${NET_BASE_DIR}/adapter/include/lwip
    ${NET_BASE_DIR}/adapter/include/
    ${NET_BASE_DIR}/lwip/src/include
)
target_compile_options(sys_arch PUBLIC 
    -Wno-cast-function-type
    -Wno-unused-variable
    -Wno-int-conversion
)

list(APPEND ALL_OBJECT_LIBRARYS driverif netdb sockets sys_arch)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
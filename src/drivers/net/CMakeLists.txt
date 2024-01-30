set(NET_SOURCE utils/net_lock.c)

if("${CONFIG_CONFIG_NET_DM90x0}")
    list(APPEND NET_SOURCE dm90x0.c)
endif()

if("${NET_SOURCE}" STREQUAL "")
    RETURN()
endif()

add_library(nuttx_net OBJECT ${NET_SOURCE})

target_include_directories(nuttx_net PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NET_BASE_DIR}/adapter/include
    ${NET_BASE_DIR}/lwip/src/include
)

target_compile_options(nuttx_net PUBLIC 
    -Wno-missing-field-initializers
    -Wno-unused-function
)

list(APPEND ALL_OBJECT_LIBRARYS nuttx_net)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
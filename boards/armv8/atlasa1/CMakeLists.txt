include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/atlasa1
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/uart
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include
)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:uart>)
add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})
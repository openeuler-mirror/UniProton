include(${HOME_PATH}/build/cmake/functions/uniproton_functions.cmake)
import_kconfig(${CONFIG_FILE_PATH}/defconfig)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp/raspi4
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/openamp
    ${HOME_PATH}/src/component/mica
    ${HOME_PATH}/src/utility/lib/include
    ${HOME_PATH}/platform/libboundscheck/include
    ${HOME_PATH}/src/include/uapi/
    ${HOME_PATH}/build/uniproton_config/config_${KCONF_DIR}/
    ${HOME_PATH}/build/open-amp/output/usr/local/include
    ${HOME_PATH}/build/libmetal/output/usr/local/include
    ${HOME_PATH}/src/libc/musl/include
    ${HOME_PATH}/src/libc/litelibc/include
)

add_subdirectory(bsp)
add_subdirectory(config)
add_subdirectory(apps)

add_subdirectory(${HOME_PATH}/src/component/proxy proxy)
add_subdirectory(${HOME_PATH}/src/component/mica mica)
list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:proxy> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:mica>)
add_library(${CPU_TYPE_UP}_APPS STATIC ${OBJS})

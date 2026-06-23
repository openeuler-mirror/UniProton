if(NOT "${CONFIG_OS_SUPPORT_OTA}" STREQUAL "y")
    return()
endif()

set(SRC
    sota/sota.c
    sota/sota_hal.c
    package/package.c
    package/package_writer.c
    package/package_head.c
    package/package_checksum.c
    flag_operate/flag_manager.c
    flag_operate/upgrade_flag.c
    utility/ota_crc.c
)

add_library(ota_component OBJECT ${SRC})

list(APPEND ALL_OBJECT_LIBRARYS ota_component)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

target_include_directories(ota_component PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sota
    ${CMAKE_CURRENT_SOURCE_DIR}/package
    ${CMAKE_CURRENT_SOURCE_DIR}/flag_operate
    ${CMAKE_CURRENT_SOURCE_DIR}/utility
)
target_compile_options(ota_component PUBLIC
    -DIOT_OTA_ENABLE_OTA
    -DPACK_CHECKSUM=PACK_NO_CHECKSUM
)

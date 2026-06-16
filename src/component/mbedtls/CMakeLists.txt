cmake_minimum_required(VERSION 3.14.1)

if(NOT "${CONFIG_OS_SUPPORT_MBEDTLS}" STREQUAL "y")
    return()
endif()

set(MBEDTLS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-2.16.8 CACHE PATH "mbedtls source root")

if(NOT EXISTS ${MBEDTLS_ROOT})
    message("--- Download mbedtls v2.16.8")
    include(FetchContent)
    FetchContent_Declare(
        mbedtls_src
        GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
        GIT_TAG v2.16.8
        SOURCE_DIR ${MBEDTLS_ROOT}
    )
    FetchContent_Populate(mbedtls_src)
    message("--- End download mbedtls")
endif()

if (NOT EXISTS ${MBEDTLS_ROOT}/include/mbedtls/ssl.h)
    message(FATAL_ERROR "Invalid MBEDTLS_ROOT: ${MBEDTLS_ROOT}")
endif()

set(MBEDTLS_SRC
    ${MBEDTLS_ROOT}/library/aes.c
    ${MBEDTLS_ROOT}/library/cipher.c
    ${MBEDTLS_ROOT}/library/cipher_wrap.c
    ${MBEDTLS_ROOT}/library/ctr_drbg.c
    ${MBEDTLS_ROOT}/library/entropy.c
    ${MBEDTLS_ROOT}/library/entropy_poll.c
    ${MBEDTLS_ROOT}/library/error.c
    ${MBEDTLS_ROOT}/library/md.c
    ${MBEDTLS_ROOT}/library/md_wrap.c
    ${MBEDTLS_ROOT}/library/platform.c
    ${MBEDTLS_ROOT}/library/platform_util.c
    ${MBEDTLS_ROOT}/library/sha256.c
    ${MBEDTLS_ROOT}/library/ssl_ciphersuites.c
    ${MBEDTLS_ROOT}/library/ssl_srv.c
    ${MBEDTLS_ROOT}/library/ssl_tls.c
    ${MBEDTLS_ROOT}/library/timing.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/entropy_hardware_poll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/net_sockets_uniproton.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/timing_alt.c
)

add_library(mbedtls_component OBJECT ${MBEDTLS_SRC})

add_library(mbedtls_config INTERFACE)

target_compile_definitions(mbedtls_config INTERFACE
    MBEDTLS_CONFIG_FILE="uniproton_mbedtls_config.h"
)

target_include_directories(mbedtls_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/port
    ${MBEDTLS_ROOT}/include
    ${HOME_PATH}/src/include/uapi
    ${HOME_PATH}/demos/sd3403/config
    ${HOME_PATH}/demos/sd3403/include
    ${HOME_PATH}/demos/sd3403/include/libc/include
    ${HOME_PATH}/output/libc/include
)

target_link_libraries(mbedtls_component PUBLIC mbedtls_config)

if (${APP} STREQUAL "mbedtls")
    set(ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_dtls_server_test.c
    )
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(mbedtlsTest OBJECT ${ALL_SRC})
    target_include_directories(mbedtlsTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(mbedtlsTest PUBLIC mbedtls_config)
endif()

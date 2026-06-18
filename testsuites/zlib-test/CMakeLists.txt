if (${APP} STREQUAL "zlib")
    set(ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib_test.c
    )
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(zlibTest OBJECT ${ALL_SRC})
    target_include_directories(zlibTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/component/zlib/include
    )
endif()

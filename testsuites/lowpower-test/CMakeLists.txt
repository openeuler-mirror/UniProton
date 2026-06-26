if (${APP} STREQUAL "lowpower")
    set(ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/lowpower_test.c
    )
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(lowpowerTest OBJECT ${ALL_SRC})
    target_include_directories(lowpowerTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

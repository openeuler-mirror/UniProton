if (NOT "${APP}" STREQUAL "cmsis")
    return()
endif()

if ("${CPU_TYPE}" STREQUAL "sd3403")
    set(CMSIS_VERSION_HEADER ${HOME_PATH}/src/component/cmsis/cmsis_os.h)
    file(READ ${CMSIS_VERSION_HEADER} CMSIS_VERSION_HEADER_CONTENT)
    string(REGEX MATCH "#[ \t]*define[ \t]+CMSIS_OS_VER[ \t]+([12])" CMSIS_VERSION_MATCH "${CMSIS_VERSION_HEADER_CONTENT}")
    if (NOT CMSIS_VERSION_MATCH)
        message(FATAL_ERROR "CMSIS_OS_VER must be defined as 1 or 2 in ${CMSIS_VERSION_HEADER}")
    endif()
    set(CMSIS_OS_VER ${CMAKE_MATCH_1})
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMSIS_VERSION_HEADER})

    if (CMSIS_OS_VER EQUAL 1)
        set(CMSIS_TEST_SRC
            v1/cmsis_v1_basic_test.c
            v1/cmsis_v1_sem_e2e_test.c
            v1/cmsis_v1_mutex_e2e_test.c
            v1/cmsis_v1_msgq_e2e_test.c
            v1/cmsis_v1_signal_e2e_test.c
            v1/cmsis_v1_timer_e2e_test.c
            v1/cmsis_v1_priority_e2e_test.c
            v1/cmsis_v1_suspend_resume_e2e_test.c
        )
    else()
        set(CMSIS_TEST_SRC
            v2/cmsis_v2_basic_test.c
            v2/cmsis_sem_e2e_test.c
            v2/cmsis_mutex_e2e_test.c
            v2/cmsis_msgq_e2e_test.c
            v2/cmsis_event_flags_e2e_test.c
            v2/cmsis_thread_flags_e2e_test.c
            v2/cmsis_timer_e2e_test.c
            v2/cmsis_priority_e2e_test.c
            v2/cmsis_suspend_resume_e2e_test.c
            v2/cmsis_kernel_lock_e2e_test.c
        )
    endif()

    add_library(cmsisTest OBJECT ${CMSIS_TEST_SRC})
    target_include_directories(cmsisTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${HOME_PATH}/src/component/cmsis
        ${HOME_PATH}/src/include/uapi
    )
endif()

if ((NOT ${APP} STREQUAL "UniPorton_test_sem") AND
    (NOT ${APP} STREQUAL "UniPorton_test_rr_sched") AND
    (NOT ${APP} STREQUAL "UniPorton_test_mmu"))
        return()
endif()

include_directories(
    ${HOME_PATH}/src/om/include
    ${HOME_PATH}/src/mem/include
)

if (${APP} STREQUAL "UniPorton_test_sem")
    set(BUILD_APP "UniPorton_test_sem")
    set(ALL_SRC sem_mutex_test.c kern_test_public.c)
endif()

if (${APP} STREQUAL "UniPorton_test_rr_sched")
    set(BUILD_APP "UniPorton_test_rr_sched")
    set(ALL_SRC sched_rr_test.c kern_test_public.c)
endif()

if (${APP} STREQUAL "UniPorton_test_mmu")
    if(${CONFIG_OS_ARCH_ARMV8})
        set(BUILD_APP "UniPorton_test_mmu")
        set(ALL_SRC mmu_test.c kern_test_public.c)
    else()
        return()
    endif()
endif()

add_library(kernTest OBJECT ${ALL_SRC})

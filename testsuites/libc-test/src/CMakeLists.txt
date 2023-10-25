set(TestCaseName UniPorton_test_posix_extra_interface)
include_directories(common)

set(SRCS common/mtest.c common/print.c common/rand.c)
add_library(common_mtest OBJECT ${SRCS})

list(APPEND OBJS 
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
    $<TARGET_OBJECTS:common_mtest>
)

file(GLOB ALL_MATH_SRC math/*.c)
list(REMOVE_ITEM ALL_MATH_SRC
${CMAKE_CURRENT_SOURCE_DIR}/math/asin.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/asinl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/atan.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/atanl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/cos.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/cosh.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/coshl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/cosl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/exp.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/expl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/fma.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/fmal.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/fmod.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/fmodl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/log.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/logl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/pow.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/powf.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/powl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/sin.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/sinl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/sinh.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/sinhl.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/tan.c # 用例数组过大, 已拆分
${CMAKE_CURRENT_SOURCE_DIR}/math/tanl.c # 用例数组过大, 已拆分
)

set(ALL_STDLIB_SRC 
    functional/qsort.c 
    functional/wcstol.c
    functional/strtol.c
    functional/strtod.c
    functional/div.c
    functional/fcvt.c
    functional/llabs.c
    functional/atof.c
    functional/imaxabs.c
    functional/lldiv.c
    functional/ldiv.c
    functional/abs.c
    functional/wcstod.c
    functional/atoll.c
    functional/imaxdiv.c
    functional/atoi.c
    functional/atol.c
    functional/labs.c
    functional/ecvt.c
    functional/gcvt.c
    functional/bsearch.c
)

file(GLOB ALL_IPC_SRC functional/ipc_msg.c functional/ipc_sem.c functional/ipc_shm.c)
file(GLOB ALL_STRING_SRC functional/string*.c)
file(GLOB ALL_REGEX_SRC regression/regex*.c)
file(GLOB ALL_PRNG_SRC functional/random.c)

if(${APP} STREQUAL "UniPorton_test_posix_math_interface")
    set(BUILD_APP "UniPorton_test_posix_math_interface")
    set(ALL_SRC runMathTest.c ${ALL_MATH_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_stdlib_interface")
    set(BUILD_APP "UniPorton_test_posix_stdlib_interface")
    set(ALL_SRC runStdlibTest.c ${ALL_STDLIB_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_ipc_interface")
    set(BUILD_APP "UniPorton_test_posix_ipc_interface")
    set(ALL_SRC runIpcTest.c ${ALL_IPC_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_string_interface")
    set(BUILD_APP "UniPorton_test_posix_string_interface")
    set(ALL_SRC runStringTest.c ${ALL_STRING_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_regex_interface")
    set(BUILD_APP "UniPorton_test_posix_regex_interface")
    set(ALL_SRC runRegexTest.c ${ALL_REGEX_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_prng_interface")
    set(BUILD_APP "UniPorton_test_posix_prng_interface")
    set(ALL_SRC runPrngTest.c ${ALL_PRNG_SRC})
else()
    return()
endif()

if(${APP} STREQUAL "UniPorton_test_posix_math_interface")
    set(ALL_EXE "")
    foreach(MATH_SRC ${ALL_MATH_SRC})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/math/" "" FILE_NAME ${MATH_SRC})
        string(REPLACE ".c" "_test" RUN_TEST ${FILE_NAME})
        add_executable("UniPorton_test_posix_math_${RUN_TEST}" ${MATH_SRC} runMathTest.c ${CXX_LIB} ${OBJS})
        string(TOUPPER ${RUN_TEST} DEF_TEST)
        target_compile_definitions("UniPorton_test_posix_math_${RUN_TEST}" PRIVATE ${DEF_TEST})
        target_link_libraries("UniPorton_test_posix_math_${RUN_TEST}" PUBLIC testsuite_support)
        list(APPEND ALL_EXE "UniPorton_test_posix_math_${RUN_TEST}")
    endforeach()
    if(${CPU_TYPE} STREQUAL "m4")
        add_custom_target(${BUILD_APP} ALL COMMAND echo 'build done' DEPENDS  ${ALL_EXE})
    elseif(${CPU_TYPE} STREQUAL "x86_64")
        add_library(posixTest OBJECT ${ALL_SRC})
    endif()
else()
    if(${CPU_TYPE} STREQUAL "m4")
        add_executable(${BUILD_APP} ${ALL_SRC} ${CXX_LIB} ${OBJS})
        target_link_libraries(${BUILD_APP} PUBLIC testsuite_support)
    elseif(${CPU_TYPE} STREQUAL "x86_64")
        add_library(posixTest OBJECT ${ALL_SRC})
    endif()
endif()

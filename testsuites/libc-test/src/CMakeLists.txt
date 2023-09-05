set(TestCaseName UniPorton_test_posix_math_interface)
include_directories(common)

set(SRCS common/mtest.c common/print.c)
add_library(common_mtest OBJECT ${SRCS})

file(GLOB ALL_MATH_SRC math/*.c)

list(APPEND OBJS 
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
    $<TARGET_OBJECTS:common_mtest>
)

if(NOT ${APP} STREQUAL "UniPorton_test_posix_math_interface")
    return()
endif()

set(BUILD_APP "UniPorton_test_posix_math_interface")
set(ALL_SRC runMathTest.c ${ALL_MATH_SRC})

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

add_custom_target(${BUILD_APP} ALL COMMAND echo 'build done' DEPENDS  ${ALL_EXE})
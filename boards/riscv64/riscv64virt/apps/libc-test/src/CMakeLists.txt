# if your libc test subdirectory has been changed you should change this
# we don't need you to keep libc-test subdirectory's father dir fixed 
# but we need that the structure of libc-test shoule keep 
set(LIBC_TST_PATH "${HOME_PATH}/testsuites/libc-test/src")
include_directories(${LIBC_TST_PATH}/common)

set(SRCS ${LIBC_TST_PATH}/common/mtest.c ${LIBC_TST_PATH}/common/print.c ${LIBC_TST_PATH}/common/rand.c)

file(GLOB ALL_MATH_SRC ${LIBC_TST_PATH}/math/*.c)
list(REMOVE_ITEM ALL_MATH_SRC
${LIBC_TST_PATH}/math/asin.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/asinl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/atan.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/atanl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/cos.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/cosh.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/coshl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/cosl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/exp.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/expl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/fma.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/fmal.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/fmod.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/fmodl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/log.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/logl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/pow.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/powf.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/powl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/sin.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/sinl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/sinh.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/sinhl.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/tan.c # 用例数组过大, 已拆分
${LIBC_TST_PATH}/math/tanl.c # 用例数组过大, 已拆分
)

set(ALL_STDLIB_SRC 
${LIBC_TST_PATH}/functional/qsort.c 
${LIBC_TST_PATH}/functional/wcstol.c
${LIBC_TST_PATH}/functional/strtol.c
${LIBC_TST_PATH}/functional/strtod.c
${LIBC_TST_PATH}/functional/div.c
${LIBC_TST_PATH}/functional/fcvt.c
${LIBC_TST_PATH}/functional/llabs.c
${LIBC_TST_PATH}/functional/atof.c
${LIBC_TST_PATH}/functional/imaxabs.c
${LIBC_TST_PATH}/functional/lldiv.c
${LIBC_TST_PATH}/functional/ldiv.c
${LIBC_TST_PATH}/functional/abs.c
${LIBC_TST_PATH}/functional/wcstod.c
${LIBC_TST_PATH}/functional/atoll.c
${LIBC_TST_PATH}/functional/imaxdiv.c
${LIBC_TST_PATH}/functional/atoi.c
${LIBC_TST_PATH}/functional/atol.c
${LIBC_TST_PATH}/functional/labs.c
${LIBC_TST_PATH}/functional/ecvt.c
${LIBC_TST_PATH}/functional/gcvt.c
${LIBC_TST_PATH}/functional/bsearch.c
)

file(GLOB ALL_IPC_SRC ${LIBC_TST_PATH}/functional/ipc_msg.c ${LIBC_TST_PATH}/functional/ipc_sem.c ${LIBC_TST_PATH}/functional/ipc_shm.c)
file(GLOB ALL_STRING_SRC ${LIBC_TST_PATH}/functional/string*.c)
file(GLOB ALL_REGEX_SRC ${LIBC_TST_PATH}/regression/regex*.c)
file(GLOB ALL_PRNG_SRC ${LIBC_TST_PATH}/functional/random.c)


if(${APP} STREQUAL "UniProton_test_posix_math_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runMathTest.c ${ALL_MATH_SRC} ${SRCS})
elseif(${APP} STREQUAL "UniProton_test_posix_stdlib_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runStdlibTest.c ${ALL_STDLIB_SRC} ${SRCS})
elseif(${APP} STREQUAL "UniProton_test_posix_ipc_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runIpcTest.c ${ALL_IPC_SRC} ${SRCS})
elseif(${APP} STREQUAL "UniProton_test_posix_string_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runStringTest.c ${ALL_STRING_SRC} ${SRCS})
elseif(${APP} STREQUAL "UniProton_test_posix_regex_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runRegexTest.c ${ALL_REGEX_SRC} ${SRCS})
elseif(${APP} STREQUAL "UniProton_test_posix_prng_interface")
    set(ALL_SRC ${LIBC_TST_PATH}/runPrngTest.c ${ALL_PRNG_SRC} ${SRCS})
else()
    return()
endif()

if(${APP} STREQUAL "UniProton_test_posix_math_interface")
    foreach(MATH_SRC ${ALL_MATH_SRC})
        string(REPLACE "${LIBC_TST_PATH}/math/" "" FILE_NAME ${MATH_SRC})
        string(REPLACE ".c" "_test" RUN_TEST ${FILE_NAME})
        string(TOUPPER ${RUN_TEST} DEF_TEST)
 	add_definitions(-D${DEF_TEST})
    endforeach()
endif()
message("all files : ${ALL_SRC}")
add_library(app OBJECT ${ALL_SRC} main.c)

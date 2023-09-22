set(TestCaseName UniPorton_test_posix_math_interface)

list(APPEND OBJS 
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
)

if(NOT ${APP} STREQUAL "UniPorton_test_shell_interface")
    return()
endif()

file(GLOB TEST_SHELL_SRC src/*.c)

set(BUILD_APP "UniPorton_test_shell_interface")

add_executable(${BUILD_APP} ${TEST_SHELL_SRC} ${CXX_LIB} ${OBJS})
target_include_directories(${BUILD_APP} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OUTPUT_PATH}/drivers/include/
    ${OUTPUT_PATH}/shell/include/
    ${OUTPUT_PATH}/shell/adapter/include
)
target_compile_options(${BUILD_APP} PUBLIC -DPRINT_USE_UART)

set(LD_OPTS )
list(APPEND LD_OPTS
    -uhelp_shellcmd
    -ulog_shellcmd
)
target_link_options(${BUILD_APP} PUBLIC ${LD_OPTS})

target_link_libraries(${BUILD_APP} PUBLIC testsuite_support)
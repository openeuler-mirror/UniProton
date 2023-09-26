set(TestCaseName UniPorton_test_drivers_interface)
file(GLOB ALL_DRIVERS_INODE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inode/*.c
)

file(GLOB ALL_DRIVERS_UART_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/uart/*.c
)

list(APPEND OBJS 
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
)

if (${APP} STREQUAL "UniPorton_test_drivers_inode_interface")
    set(BUILD_APP "UniPorton_test_drivers_inode_interface")
    set(ALL_SRC runInodeTest.c ${ALL_DRIVERS_INODE_SRC})
elseif (${APP} STREQUAL "UniPorton_test_drivers_uart_interface")
    set(BUILD_APP "UniPorton_test_drivers_uart_interface")
    set(ALL_SRC runUartTest.c ${ALL_DRIVERS_UART_SRC})
else()
    return()
endif()

add_executable(${BUILD_APP} ${ALL_SRC} ${CXX_LIB} ${OBJS})
target_link_libraries(${BUILD_APP} PUBLIC testsuite_support)
target_include_directories(${BUILD_APP} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${OUTPUT_PATH}/drivers/include/
)
if (${APP} STREQUAL "UniPorton_test_drivers_uart_interface")
target_include_directories(${BUILD_APP} PUBLIC 
    ${UNIPROTON_PROJECT_DIR}/src/drivers/include
    ${UNIPROTON_PROJECT_DIR}/src/drivers/include/arch/arm/common
    ${UNIPROTON_PROJECT_DIR}/src/drivers/include/arch/arm/stm32
    ${UNIPROTON_PROJECT_DIR}/src/drivers/base/
)
endif()
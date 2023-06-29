set(TestCaseName UniProton_test_deadlock-break)
set(TestCaseSources
    deadlock-break.c
)

list(APPEND OBJS
    $<TARGET_OBJECTS:bsp>
    $<TARGET_OBJECTS:config>
)

add_executable(${TestCaseName} ${CXX_LIB} ${OBJS} ${TestCaseSources})
target_link_libraries(${TestCaseName} PUBLIC rhealstone_support)
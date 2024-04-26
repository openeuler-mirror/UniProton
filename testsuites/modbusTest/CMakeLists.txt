if (NOT ${APP} STREQUAL "UniProton_modbus_demo")
    return()
endif()

set(BUILD_APP "UniProton_modbus_demo")
set(ALL_SRC
    modbus_unit_test_client.c
)

if(${CPU_TYPE} STREQUAL "x86_64")
    add_library(modbusTest OBJECT ${ALL_SRC})
endif()
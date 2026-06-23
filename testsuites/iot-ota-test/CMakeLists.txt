set(ALL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/iot_ota_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/iot_ota_test_common.c
)

set(IOT_OTA_TEST_ENABLED FALSE)
if ("${CONFIG_OS_SUPPORT_OTA}" STREQUAL "y")
    list(APPEND ALL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/ota/iot_ota_test_ota.c)
    set(IOT_OTA_TEST_ENABLED TRUE)
endif()

if ("${CONFIG_OS_SUPPORT_LWM2M}" STREQUAL "y")
    list(APPEND ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m/iot_ota_test_lwm2m.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m/iot_lwm2m_core_client.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m/lwm2m_demo_client.c
    )
    set(IOT_OTA_TEST_ENABLED TRUE)
endif()

if ("${CONFIG_OS_SUPPORT_MQTT}" STREQUAL "y")
    list(APPEND ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt/iot_ota_test_mqtt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt/mqtt_demo_client.c
    )
    set(IOT_OTA_TEST_ENABLED TRUE)
endif()

if ("${CONFIG_OS_SUPPORT_NB_IOT}" STREQUAL "y")
    list(APPEND ALL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/nb_iot/iot_ota_test_nb_iot.c)
    set(IOT_OTA_TEST_ENABLED TRUE)
endif()

if (("${CONFIG_OS_SUPPORT_OTA}" STREQUAL "y") AND ("${CONFIG_OS_SUPPORT_LWM2M}" STREQUAL "y"))
    list(APPEND ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_ota/iot_ota_test_lwm2m_ota.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_ota/lwm2m_ota_client.c
    )
endif()

if (("${CONFIG_OS_SUPPORT_OTA}" STREQUAL "y") AND ("${CONFIG_OS_SUPPORT_MQTT}" STREQUAL "y"))
    list(APPEND ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt_ota/iot_ota_test_mqtt_ota.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt_ota/mqtt_ota_client.c
    )
endif()

if (NOT IOT_OTA_TEST_ENABLED)
    message(FATAL_ERROR "PLEASE ENABLE AT LEAST ONE OF CONFIG_OS_SUPPORT_OTA/LWM2M/MQTT/NB_IOT")
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(iotOtaTest OBJECT ${ALL_SRC})
    target_include_directories(iotOtaTest PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt
        ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_ota
        ${CMAKE_CURRENT_SOURCE_DIR}/mqtt_ota
        ${HOME_PATH}/src/component/ota/include
        ${HOME_PATH}/src/component/ota/sota
        ${HOME_PATH}/src/component/ota/flag_operate
        ${HOME_PATH}/src/component/ota/package
        ${HOME_PATH}/src/component/ota/utility
        ${HOME_PATH}/src/component/connectivity
        ${HOME_PATH}/src/component/connectivity/mqtt/MQTTPacket/src
        ${HOME_PATH}/src/component/connectivity/mqtt/MQTTClient-C/src
        ${HOME_PATH}/src/component/connectivity/mqtt/MQTTClient-C/src/uniproton
        ${HOME_PATH}/src/component/connectivity/lwm2m/core
        ${HOME_PATH}/src/component/connectivity/lwm2m/core/er-coap-13
    )
    target_compile_options(iotOtaTest PUBLIC -DPACK_CHECKSUM=PACK_NO_CHECKSUM -DLWM2M_CLIENT_MODE -DLWM2M_LITTLE_ENDIAN)
endif()

set(SRC)
set(CONNECTIVITY_COMPILE_OPTIONS)

if ("${CONFIG_OS_SUPPORT_LWM2M}" STREQUAL "y")
    list(APPEND SRC
        lwm2m/core/block1.c
        lwm2m/core/data.c
        lwm2m/core/discover.c
        lwm2m/core/er-coap-13/er-coap-13.c
        lwm2m/core/liblwm2m.c
        lwm2m/core/list.c
        lwm2m/core/lwm2m_uniproton_port.c
        lwm2m/core/lwm2m_utils.c
        lwm2m/core/management.c
        lwm2m/core/objects.c
        lwm2m/core/observe.c
        lwm2m/core/packet.c
        lwm2m/core/registration.c
        lwm2m/core/tlv.c
        lwm2m/core/transaction.c
        lwm2m/core/uri.c
    )
    list(APPEND CONNECTIVITY_COMPILE_OPTIONS -DIOT_OTA_ENABLE_LWM2M -DLWM2M_CLIENT_MODE -DLWM2M_LITTLE_ENDIAN)
endif()

if ("${CONFIG_OS_SUPPORT_MQTT}" STREQUAL "y")
    list(APPEND SRC
        mqtt/MQTTPacket/src/MQTTConnectClient.c
        mqtt/MQTTPacket/src/MQTTDeserializePublish.c
        mqtt/MQTTPacket/src/MQTTPacket.c
        mqtt/MQTTPacket/src/MQTTSerializePublish.c
        mqtt/MQTTPacket/src/MQTTSubscribeClient.c
        mqtt/MQTTPacket/src/MQTTUnsubscribeClient.c
        mqtt/MQTTClient-C/src/MQTTClient.c
        mqtt/MQTTClient-C/src/uniproton/MQTTuniproton.c
    )
    list(APPEND CONNECTIVITY_COMPILE_OPTIONS -DIOT_OTA_ENABLE_MQTT)
endif()

if ("${CONFIG_OS_SUPPORT_NB_IOT}" STREQUAL "y")
    list(APPEND SRC
        nb_iot/nb_direct_link.c
        nb_iot/nb_hw_direct.c
        nb_iot/los_nb_api.c
    )
    list(APPEND CONNECTIVITY_COMPILE_OPTIONS -DIOT_OTA_ENABLE_NB_IOT)
endif()

if (NOT SRC)
    return()
endif()

add_library(connectivity_component OBJECT ${SRC})

list(APPEND ALL_OBJECT_LIBRARYS connectivity_component)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

target_include_directories(connectivity_component PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mqtt/MQTTPacket/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mqtt/MQTTClient-C/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mqtt/MQTTClient-C/src/uniproton
    ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m/core
    ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m/core/er-coap-13
)
target_compile_options(connectivity_component PUBLIC ${CONNECTIVITY_COMPILE_OPTIONS})

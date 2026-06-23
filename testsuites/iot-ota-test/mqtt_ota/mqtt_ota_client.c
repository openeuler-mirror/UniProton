#include "mqtt_ota_client.h"

#include <stdio.h>
#include <string.h>

#include "MQTTClient.h"

#define MQTT_TOPIC_REQUEST "ota/request"
#define MQTT_TOPIC_CHUNK "ota/chunk"
#define MQTT_TOPIC_MANIFEST_REQUEST "ota/manifest/request"
#define MQTT_TOPIC_MANIFEST "ota/manifest"
#define MQTT_OTA_VERSION "2.0.0"
#define MQTT_COMMAND_TIMEOUT_MS 3000U
#define MQTT_YIELD_TIMEOUT_MS 1000
#define MQTT_YIELD_RETRY 5U

typedef struct {
    uint8_t *buf;
    uint32_t expected_offset;
    uint32_t expected_len;
    uint32_t received_len;
    uint32_t manifest_len;
    uint32_t manifest_crc;
    int manifest_ok;
} mqtt_chunk_context_s;

static mqtt_chunk_context_s g_mqtt_chunk;

static uint32_t mqtt_crc32(const uint8_t *buf, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    uint32_t i;
    uint32_t bit;

    for (i = 0; i < len; i++) {
        crc ^= buf[i];
        for (bit = 0; bit < 8U; bit++) {
            if ((crc & 1U) != 0U) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

static int mqtt_topic_equals(MQTTString *topic, const char *expected)
{
    int len;
    const char *data;

    if (topic == NULL || expected == NULL) {
        return 0;
    }
    if (topic->cstring != NULL) {
        return strcmp(topic->cstring, expected) == 0;
    }
    len = topic->lenstring.len;
    data = topic->lenstring.data;
    return data != NULL && len == (int)strlen(expected) && memcmp(data, expected, (size_t)len) == 0;
}

static void mqtt_chunk_handler(MessageData *data)
{
    MQTTMessage *message;
    char header[64];
    char version[32];
    unsigned int chunk_offset;
    unsigned int chunk_size;
    unsigned int chunk_crc;
    unsigned int package_size;
    unsigned int package_crc;
    uint8_t *payload;
    int header_len;

    if (data == NULL) {
        return;
    }
    message = data->message;
    if (message == NULL || message->payload == NULL) {
        return;
    }
    if (mqtt_topic_equals(data->topicName, MQTT_TOPIC_CHUNK)) {
        payload = (uint8_t *)memchr(message->payload, '\n', (size_t)message->payloadlen);
        if (payload == NULL) {
            return;
        }
        header_len = (int)(payload - (uint8_t *)message->payload);
        if (header_len <= 0 || header_len >= (int)sizeof(header)) {
            return;
        }
        (void)memcpy(header, message->payload, (size_t)header_len);
        header[header_len] = '\0';
        payload++;
        if (sscanf(header, "CHUNK %u %u %x", &chunk_offset, &chunk_size, &chunk_crc) != 3) {
            return;
        }
        if ((uint32_t)chunk_offset != g_mqtt_chunk.expected_offset ||
            (uint32_t)chunk_size != g_mqtt_chunk.expected_len ||
            message->payloadlen - header_len - 1 != (int)g_mqtt_chunk.expected_len) {
            return;
        }
        if (mqtt_crc32(payload, g_mqtt_chunk.expected_len) != (uint32_t)chunk_crc) {
            return;
        }
        (void)memcpy(g_mqtt_chunk.buf, payload, g_mqtt_chunk.expected_len);
        g_mqtt_chunk.received_len = g_mqtt_chunk.expected_len;
        return;
    }
    if (mqtt_topic_equals(data->topicName, MQTT_TOPIC_MANIFEST)) {
        char text[96];
        int copy_len = (message->payloadlen < (int)sizeof(text) - 1) ? message->payloadlen : (int)sizeof(text) - 1;

        (void)memcpy(text, message->payload, (size_t)copy_len);
        text[copy_len] = '\0';
        if (sscanf(text, "MANIFEST %31s %u %x", version, &package_size, &package_crc) == 3 &&
            strcmp(version, MQTT_OTA_VERSION) == 0) {
            g_mqtt_chunk.manifest_len = (uint32_t)package_size;
            g_mqtt_chunk.manifest_crc = (uint32_t)package_crc;
            g_mqtt_chunk.manifest_ok = 1;
        }
    }
}

static int mqtt_publish_manifest_request(MQTTClient *client)
{
    MQTTMessage message;
    char req[] = "GET_MANIFEST";

    (void)memset(&message, 0, sizeof(message));
    message.qos = QOS0;
    message.payload = req;
    message.payloadlen = strlen(req);
    return MQTTPublish(client, MQTT_TOPIC_MANIFEST_REQUEST, &message);
}

static int mqtt_publish_request(MQTTClient *client, uint32_t offset, uint32_t len)
{
    MQTTMessage message;
    char req[48];

    (void)snprintf(req, sizeof(req), "GET_CHUNK %u %u", (unsigned int)offset, (unsigned int)len);
    (void)memset(&message, 0, sizeof(message));
    message.qos = QOS0;
    message.payload = req;
    message.payloadlen = strlen(req);
    return MQTTPublish(client, MQTT_TOPIC_REQUEST, &message);
}

static int mqtt_wait_chunk(MQTTClient *client, uint8_t *buf, uint32_t expected_offset, uint32_t expected_len)
{
    uint32_t retry;

    g_mqtt_chunk.buf = buf;
    g_mqtt_chunk.expected_offset = expected_offset;
    g_mqtt_chunk.expected_len = expected_len;
    g_mqtt_chunk.received_len = 0;

    for (retry = 0; retry < MQTT_YIELD_RETRY; retry++) {
        if (MQTTYield(client, MQTT_YIELD_TIMEOUT_MS) < 0) {
            return -1;
        }
        if (g_mqtt_chunk.received_len == expected_len) {
            return 0;
        }
    }
    return -1;
}

static int mqtt_wait_manifest(MQTTClient *client, uint32_t *package_len)
{
    uint32_t retry;

    g_mqtt_chunk.manifest_len = 0;
    g_mqtt_chunk.manifest_crc = 0;
    g_mqtt_chunk.manifest_ok = 0;
    for (retry = 0; retry < MQTT_YIELD_RETRY; retry++) {
        if (MQTTYield(client, MQTT_YIELD_TIMEOUT_MS) < 0) {
            return -1;
        }
        if (g_mqtt_chunk.manifest_ok && g_mqtt_chunk.manifest_len > 0U) {
            *package_len = g_mqtt_chunk.manifest_len;
            return 0;
        }
    }
    return -1;
}

int mqtt_ota_fetch_package(const char *host, uint16_t port, const char *client_id,
    uint8_t *package, uint32_t package_len, uint32_t chunk_size)
{
    Network network;
    MQTTClient client;
    MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
    uint8_t send_buf[256];
    uint8_t read_buf[256];
    uint32_t offset = 0;
    int ret = -1;

    if (host == NULL || port == 0U || client_id == NULL || package == NULL || package_len == 0U || chunk_size == 0U) {
        return -1;
    }

    NetworkInit(&network);
    (void)memset(&client, 0, sizeof(client));
    if (NetworkConnect(&network, (char *)host, (int)port) != 0) {
        return -1;
    }

    if (MQTTClientInit(&client, &network, MQTT_COMMAND_TIMEOUT_MS, send_buf, sizeof(send_buf),
        read_buf, sizeof(read_buf)) != MQTT_SUCCESS) {
        goto exit;
    }
    connect_data.clientID.cstring = (char *)client_id;
    if (MQTTConnect(&client, &connect_data) != MQTT_SUCCESS ||
        MQTTSubscribe(&client, MQTT_TOPIC_CHUNK, QOS0, mqtt_chunk_handler) != MQTT_SUCCESS) {
        goto exit;
    }

    while (offset < package_len) {
        uint32_t len = (package_len - offset < chunk_size) ? (package_len - offset) : chunk_size;
        if (mqtt_publish_request(&client, offset, len) != MQTT_SUCCESS ||
            mqtt_wait_chunk(&client, &package[offset], offset, len) != 0) {
            goto exit;
        }
        offset += len;
    }

    ret = 0;

exit:
    if (client.isconnected) {
        (void)MQTTDisconnect(&client);
    }
    MQTTClientDeInit(&client);
    NetworkDisconnect(&network);
    return ret;
}

int mqtt_ota_fetch_manifest_and_package(const char *host, uint16_t port, const char *client_id,
    uint8_t *package, uint32_t package_capacity, uint32_t chunk_size, uint32_t *package_len)
{
    Network network;
    MQTTClient client;
    MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
    uint8_t send_buf[256];
    uint8_t read_buf[256];
    uint32_t len = 0;
    uint32_t offset = 0;
    int ret = -1;

    if (host == NULL || port == 0U || client_id == NULL || package == NULL ||
        package_capacity == 0U || chunk_size == 0U || package_len == NULL) {
        return -1;
    }

    NetworkInit(&network);
    (void)memset(&client, 0, sizeof(client));
    if (NetworkConnect(&network, (char *)host, (int)port) != 0) {
        return -1;
    }

    if (MQTTClientInit(&client, &network, MQTT_COMMAND_TIMEOUT_MS, send_buf, sizeof(send_buf),
        read_buf, sizeof(read_buf)) != MQTT_SUCCESS) {
        goto exit;
    }
    connect_data.clientID.cstring = (char *)client_id;
    if (MQTTConnect(&client, &connect_data) != MQTT_SUCCESS ||
        MQTTSubscribe(&client, MQTT_TOPIC_MANIFEST, QOS0, mqtt_chunk_handler) != MQTT_SUCCESS ||
        MQTTSubscribe(&client, MQTT_TOPIC_CHUNK, QOS0, mqtt_chunk_handler) != MQTT_SUCCESS) {
        goto exit;
    }
    if (mqtt_publish_manifest_request(&client) != MQTT_SUCCESS || mqtt_wait_manifest(&client, &len) != 0 ||
        len == 0U || len > package_capacity) {
        goto exit;
    }

    while (offset < len) {
        uint32_t chunk_len = (len - offset < chunk_size) ? (len - offset) : chunk_size;
        if (mqtt_publish_request(&client, offset, chunk_len) != MQTT_SUCCESS ||
            mqtt_wait_chunk(&client, &package[offset], offset, chunk_len) != 0) {
            goto exit;
        }
        offset += chunk_len;
    }
    if (mqtt_crc32(package, len) != g_mqtt_chunk.manifest_crc) {
        goto exit;
    }
    *package_len = len;
    ret = 0;

exit:
    if (client.isconnected) {
        (void)MQTTDisconnect(&client);
    }
    MQTTClientDeInit(&client);
    NetworkDisconnect(&network);
    return ret;
}

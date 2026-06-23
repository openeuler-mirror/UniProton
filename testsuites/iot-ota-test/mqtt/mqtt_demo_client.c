#include "mqtt_demo_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MQTTClient.h"

#define MQTT_DEMO_TOPIC_REPORT "demo/report"
#define MQTT_DEMO_TOPIC_COMMAND "demo/command"
#define MQTT_DEMO_TOPIC_RESPONSE "demo/response"
#define MQTT_DEMO_COMMAND_TIMEOUT_MS 3000U
#define MQTT_DEMO_YIELD_TIMEOUT_MS 1000
#define MQTT_DEMO_YIELD_RETRY 10U

typedef struct {
    int received;
    int mid;
} mqtt_demo_context_s;

static mqtt_demo_context_s g_mqtt_demo;

static const char *json_find_value(const char *json, const char *key)
{
    char pattern[32];
    const char *pos;

    if (json == NULL || key == NULL || strlen(key) + 4U > sizeof(pattern)) {
        return NULL;
    }
    (void)snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return NULL;
    }
    pos += strlen(pattern);
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n') {
        pos++;
    }
    if (*pos != ':') {
        return NULL;
    }
    pos++;
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n') {
        pos++;
    }
    return pos;
}

static int json_get_string(const char *json, const char *key, char *value, uint32_t value_len)
{
    const char *pos;
    uint32_t len = 0;

    if (value == NULL || value_len == 0U) {
        return -1;
    }
    pos = json_find_value(json, key);
    if (pos == NULL || *pos != '"') {
        return -1;
    }
    pos++;
    while (pos[len] != '\0' && pos[len] != '"') {
        if (pos[len] == '\\' || len + 1U >= value_len) {
            return -1;
        }
        len++;
    }
    if (pos[len] != '"') {
        return -1;
    }
    (void)memcpy(value, pos, len);
    value[len] = '\0';
    return 0;
}

static int json_get_int(const char *json, const char *key, int *value)
{
    const char *pos;
    char *end = NULL;
    long parsed;

    if (value == NULL) {
        return -1;
    }
    pos = json_find_value(json, key);
    if (pos == NULL) {
        return -1;
    }
    parsed = strtol(pos, &end, 10);
    if (end == pos) {
        return -1;
    }
    *value = (int)parsed;
    return 0;
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

static int parse_command_json(const char *msg, int *mid)
{
    char msg_type[16];
    char cmd[32];

    if (msg == NULL || mid == NULL) {
        return -1;
    }
    if (json_get_string(msg, "msgType", msg_type, sizeof(msg_type)) != 0 || strcmp(msg_type, "cloudReq") != 0) {
        return -1;
    }
    if (json_get_string(msg, "cmd", cmd, sizeof(cmd)) != 0 || strcmp(cmd, "SetReportPeriod") != 0) {
        return -1;
    }
    if (json_get_int(msg, "mid", mid) != 0 || *mid <= 0) {
        return -1;
    }
    return 0;
}

static void mqtt_demo_command_handler(MessageData *data)
{
    MQTTMessage *message;
    char msg[160];
    uint32_t copy_len;

    if (data == NULL || !mqtt_topic_equals(data->topicName, MQTT_DEMO_TOPIC_COMMAND)) {
        return;
    }
    message = data->message;
    if (message == NULL || message->payload == NULL || message->payloadlen <= 0) {
        return;
    }
    copy_len = ((uint32_t)message->payloadlen < sizeof(msg) - 1U) ?
        (uint32_t)message->payloadlen : sizeof(msg) - 1U;
    (void)memcpy(msg, message->payload, copy_len);
    msg[copy_len] = '\0';
    if (parse_command_json(msg, &g_mqtt_demo.mid) == 0) {
        g_mqtt_demo.received = 1;
    }
}

static int mqtt_demo_publish(MQTTClient *client, const char *topic, const char *payload)
{
    MQTTMessage message;

    if (client == NULL || topic == NULL || payload == NULL) {
        return -1;
    }
    (void)memset(&message, 0, sizeof(message));
    message.qos = QOS0;
    message.payload = (void *)payload;
    message.payloadlen = strlen(payload);
    return MQTTPublish(client, topic, &message);
}

int mqtt_demo_run(const char *host, uint16_t port, const char *client_id)
{
    Network network;
    MQTTClient client;
    MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
    uint8_t send_buf[384];
    uint8_t read_buf[384];
    char response[192];
    uint32_t retry;
    int ret = -1;
    const char report[] =
        "{\"msgType\":\"deviceReq\",\"hasMore\":0,\"data\":[{\"serviceId\":\"Battery\","
        "\"serviceData\":{\"batteryLevel\":67},\"eventTime\":\"20201103T114920Z\"}]}";

    if (host == NULL || port == 0U || client_id == NULL) {
        return -1;
    }

    NetworkInit(&network);
    (void)memset(&client, 0, sizeof(client));
    (void)memset(&g_mqtt_demo, 0, sizeof(g_mqtt_demo));
    if (NetworkConnect(&network, (char *)host, (int)port) != 0) {
        return -1;
    }
    if (MQTTClientInit(&client, &network, MQTT_DEMO_COMMAND_TIMEOUT_MS, send_buf, sizeof(send_buf),
        read_buf, sizeof(read_buf)) != MQTT_SUCCESS) {
        goto exit;
    }
    connect_data.clientID.cstring = (char *)client_id;
    if (MQTTConnect(&client, &connect_data) != MQTT_SUCCESS ||
        MQTTSubscribe(&client, MQTT_DEMO_TOPIC_COMMAND, QOS0, mqtt_demo_command_handler) != MQTT_SUCCESS) {
        goto exit;
    }
    if (mqtt_demo_publish(&client, MQTT_DEMO_TOPIC_REPORT, report) != MQTT_SUCCESS) {
        goto exit;
    }
    for (retry = 0; retry < MQTT_DEMO_YIELD_RETRY; retry++) {
        if (MQTTYield(&client, MQTT_DEMO_YIELD_TIMEOUT_MS) < 0) {
            goto exit;
        }
        if (g_mqtt_demo.received) {
            break;
        }
    }
    if (!g_mqtt_demo.received) {
        goto exit;
    }
    (void)snprintf(response, sizeof(response),
        "{\"msgType\":\"deviceRsp\",\"mid\":%d,\"errcode\":0,\"hasMore\":0,\"body\":{\"result\":\"ok\"}}",
        g_mqtt_demo.mid);
    if (mqtt_demo_publish(&client, MQTT_DEMO_TOPIC_RESPONSE, response) != MQTT_SUCCESS) {
        goto exit;
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

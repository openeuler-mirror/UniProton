#include "iot_lwm2m_core_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "liblwm2m.h"
#include "lwm2m_uniproton_port.h"

#define IOT_LWM2M_SERVER_ID 123
#define IOT_LWM2M_LIFETIME 20
#define IOT_LWM2M_RES_FW_PACKAGE 0
#define IOT_LWM2M_RES_FW_PACKAGE_URI 1
#define IOT_LWM2M_RES_FW_UPDATE 2
#define IOT_LWM2M_RES_FW_STATE 3
#define IOT_LWM2M_RES_FW_RESULT 5

typedef enum {
    IOT_LWM2M_MODE_DEMO,
    IOT_LWM2M_MODE_OTA_PACKAGE,
} iot_lwm2m_mode_e;

typedef struct {
    iot_lwm2m_mode_e mode;
    int read_ok;
    int write_ok;
    int execute_ok;
    uint8_t fw_state;
    uint8_t fw_result;
    uint8_t fw_have_package;
    uint8_t fw_executed;
    int64_t current_time;
    char server_uri[64];
    char utc_offset[8];
    char timezone[32];
    uint8_t *package;
    uint32_t package_capacity;
    uint32_t package_len;
} iot_lwm2m_core_state_s;

typedef struct {
    lwm2m_list_t security_inst;
    lwm2m_list_t server_inst;
    lwm2m_list_t device_inst;
    lwm2m_list_t firmware_inst;
    lwm2m_object_t security_obj;
    lwm2m_object_t server_obj;
    lwm2m_object_t device_obj;
    lwm2m_object_t firmware_obj;
    iot_lwm2m_core_state_s *state;
} iot_lwm2m_objects_s;

static int set_recv_timeout(int fd, uint32_t timeout_ms)
{
    struct timeval tv;

    tv.tv_sec = timeout_ms / 1000U;
    tv.tv_usec = (timeout_ms % 1000U) * 1000U;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int ensure_data_array(int *numDataP, lwm2m_data_t **dataArrayP, const uint16_t *ids, int count)
{
    int i;

    if (*numDataP != 0) {
        return 0;
    }
    *dataArrayP = lwm2m_data_new(count);
    if (*dataArrayP == NULL) {
        return -1;
    }
    *numDataP = count;
    for (i = 0; i < count; i++) {
        (*dataArrayP)[i].id = ids[i];
    }
    return 0;
}

static uint8_t read_security(uint16_t instanceId, int *numDataP, lwm2m_data_t **dataArrayP,
    lwm2m_data_cfg_t *dataCfg, lwm2m_object_t *objectP)
{
    static const uint16_t ids[] = {
        LWM2M_SECURITY_BOOTSTRAP_ID,
        LWM2M_SECURITY_SHORT_SERVER_ID,
        LWM2M_SECURITY_HOLD_OFF_ID,
    };
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;
    int i;

    (void)dataCfg;
    if (instanceId != 0 || state == NULL || ensure_data_array(numDataP, dataArrayP, ids,
        (int)(sizeof(ids) / sizeof(ids[0]))) != 0) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < *numDataP; i++) {
        switch ((*dataArrayP)[i].id) {
            case LWM2M_SECURITY_URI_ID:
                lwm2m_data_encode_string(state->server_uri, &(*dataArrayP)[i]);
                break;
            case LWM2M_SECURITY_BOOTSTRAP_ID:
                lwm2m_data_encode_bool(false, &(*dataArrayP)[i]);
                break;
            case LWM2M_SECURITY_SECURITY_ID:
                lwm2m_data_encode_int(LWM2M_SECURITY_MODE_NONE, &(*dataArrayP)[i]);
                break;
            case LWM2M_SECURITY_SHORT_SERVER_ID:
                lwm2m_data_encode_int(IOT_LWM2M_SERVER_ID, &(*dataArrayP)[i]);
                break;
            case LWM2M_SECURITY_HOLD_OFF_ID:
                lwm2m_data_encode_int(0, &(*dataArrayP)[i]);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}

static uint8_t read_server(uint16_t instanceId, int *numDataP, lwm2m_data_t **dataArrayP,
    lwm2m_data_cfg_t *dataCfg, lwm2m_object_t *objectP)
{
    static const uint16_t ids[] = { LWM2M_SERVER_SHORT_ID_ID, LWM2M_SERVER_LIFETIME_ID, LWM2M_SERVER_BINDING_ID };
    int i;

    (void)dataCfg;
    (void)objectP;
    if (instanceId != 0 || ensure_data_array(numDataP, dataArrayP, ids, (int)(sizeof(ids) / sizeof(ids[0]))) != 0) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < *numDataP; i++) {
        switch ((*dataArrayP)[i].id) {
            case LWM2M_SERVER_SHORT_ID_ID:
                lwm2m_data_encode_int(IOT_LWM2M_SERVER_ID, &(*dataArrayP)[i]);
                break;
            case LWM2M_SERVER_LIFETIME_ID:
                lwm2m_data_encode_int(IOT_LWM2M_LIFETIME, &(*dataArrayP)[i]);
                break;
            case LWM2M_SERVER_BINDING_ID:
                lwm2m_data_encode_string("U", &(*dataArrayP)[i]);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}

static uint8_t read_device(uint16_t instanceId, int *numDataP, lwm2m_data_t **dataArrayP,
    lwm2m_data_cfg_t *dataCfg, lwm2m_object_t *objectP)
{
    static const uint16_t ids[] = { 0, 1, 2, 3, 13, 14, 15 };
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;
    int i;

    (void)dataCfg;
    if (instanceId != 0 || state == NULL || ensure_data_array(numDataP, dataArrayP, ids,
        (int)(sizeof(ids) / sizeof(ids[0]))) != 0) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < *numDataP; i++) {
        switch ((*dataArrayP)[i].id) {
            case 0:
                lwm2m_data_encode_string("Open Mobile Alliance", &(*dataArrayP)[i]);
                state->read_ok = 1;
                break;
            case 1:
                lwm2m_data_encode_string("Lightweight M2M Client", &(*dataArrayP)[i]);
                break;
            case 2:
                lwm2m_data_encode_string("345000123", &(*dataArrayP)[i]);
                break;
            case 3:
                lwm2m_data_encode_string("example_ver001", &(*dataArrayP)[i]);
                break;
            case 13:
                lwm2m_data_encode_int(state->current_time, &(*dataArrayP)[i]);
                break;
            case 14:
                lwm2m_data_encode_string(state->utc_offset, &(*dataArrayP)[i]);
                break;
            case 15:
                lwm2m_data_encode_string(state->timezone, &(*dataArrayP)[i]);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}

static void copy_lwm2m_string(char *dst, size_t dst_len, const lwm2m_data_t *data)
{
    size_t copy_len;

    if (dst == NULL || dst_len == 0 || data == NULL || data->value.asBuffer.buffer == NULL) {
        return;
    }
    copy_len = data->value.asBuffer.length;
    if (copy_len >= dst_len) {
        copy_len = dst_len - 1;
    }
    (void)memcpy(dst, data->value.asBuffer.buffer, copy_len);
    dst[copy_len] = '\0';
}

static uint8_t write_device(uint16_t instanceId, int numData, lwm2m_data_t *dataArray, lwm2m_object_t *objectP)
{
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;
    char text[32];
    int i;

    if (instanceId != 0 || state == NULL || dataArray == NULL) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < numData; i++) {
        switch (dataArray[i].id) {
            case 13:
                copy_lwm2m_string(text, sizeof(text), &dataArray[i]);
                state->current_time = strtoll(text, NULL, 10);
                break;
            case 14:
                copy_lwm2m_string(state->utc_offset, sizeof(state->utc_offset), &dataArray[i]);
                state->write_ok = 1;
                break;
            case 15:
                copy_lwm2m_string(state->timezone, sizeof(state->timezone), &dataArray[i]);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_204_CHANGED;
}

static uint8_t execute_device(uint16_t instanceId, uint16_t resourceId, uint8_t *buffer, int length,
    lwm2m_object_t *objectP)
{
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;

    (void)buffer;
    (void)length;
    if (instanceId != 0 || state == NULL || (resourceId != 4 && resourceId != 5)) {
        return COAP_404_NOT_FOUND;
    }
    state->execute_ok = 1;
    return COAP_204_CHANGED;
}

static uint8_t read_firmware(uint16_t instanceId, int *numDataP, lwm2m_data_t **dataArrayP,
    lwm2m_data_cfg_t *dataCfg, lwm2m_object_t *objectP)
{
    static const uint16_t ids[] = { IOT_LWM2M_RES_FW_STATE, IOT_LWM2M_RES_FW_RESULT };
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;
    int i;

    (void)dataCfg;
    if (instanceId != 0 || state == NULL || ensure_data_array(numDataP, dataArrayP, ids,
        (int)(sizeof(ids) / sizeof(ids[0]))) != 0) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < *numDataP; i++) {
        switch ((*dataArrayP)[i].id) {
            case IOT_LWM2M_RES_FW_STATE:
                lwm2m_data_encode_int(state->fw_state, &(*dataArrayP)[i]);
                break;
            case IOT_LWM2M_RES_FW_RESULT:
                lwm2m_data_encode_int(state->fw_result, &(*dataArrayP)[i]);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}

static uint8_t write_firmware(uint16_t instanceId, int numData, lwm2m_data_t *dataArray, lwm2m_object_t *objectP)
{
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;
    size_t length;
    int i;

    if (instanceId != 0 || state == NULL || dataArray == NULL) {
        return COAP_404_NOT_FOUND;
    }
    for (i = 0; i < numData; i++) {
        if (dataArray[i].id == IOT_LWM2M_RES_FW_PACKAGE || dataArray[i].id == IOT_LWM2M_RES_FW_PACKAGE_URI) {
            length = dataArray[i].value.asBuffer.length;
            if (state->package == NULL || length == 0 || length > state->package_capacity) {
                state->fw_result = 8U;
                return COAP_400_BAD_REQUEST;
            }
            (void)memcpy(state->package, dataArray[i].value.asBuffer.buffer, length);
            state->package_len = (uint32_t)length;
            state->fw_have_package = 1U;
            state->fw_state = 1U;
            state->fw_result = 0U;
            return COAP_204_CHANGED;
        }
    }
    return COAP_404_NOT_FOUND;
}

static uint8_t execute_firmware(uint16_t instanceId, uint16_t resourceId, uint8_t *buffer, int length,
    lwm2m_object_t *objectP)
{
    iot_lwm2m_core_state_s *state = (iot_lwm2m_core_state_s *)objectP->userData;

    (void)buffer;
    (void)length;
    if (instanceId != 0 || resourceId != IOT_LWM2M_RES_FW_UPDATE || state == NULL || state->fw_have_package == 0U) {
        if (state != NULL) {
            state->fw_result = 8U;
        }
        return COAP_400_BAD_REQUEST;
    }
    state->fw_executed = 1U;
    state->fw_state = 2U;
    state->fw_result = 0U;
    return COAP_204_CHANGED;
}

static void init_object(lwm2m_object_t *objectP, lwm2m_list_t *instanceP, uint16_t object_id, void *userData)
{
    (void)memset(objectP, 0, sizeof(*objectP));
    (void)memset(instanceP, 0, sizeof(*instanceP));
    objectP->objID = object_id;
    objectP->instanceList = instanceP;
    objectP->userData = userData;
    instanceP->id = 0;
}

static void init_objects(iot_lwm2m_objects_s *objects, iot_lwm2m_core_state_s *state)
{
    (void)memset(objects, 0, sizeof(*objects));
    objects->state = state;
    init_object(&objects->security_obj, &objects->security_inst, LWM2M_SECURITY_OBJECT_ID, state);
    objects->security_obj.readFunc = read_security;
    init_object(&objects->server_obj, &objects->server_inst, LWM2M_SERVER_OBJECT_ID, state);
    objects->server_obj.readFunc = read_server;
    init_object(&objects->device_obj, &objects->device_inst, LWM2M_DEVICE_OBJECT_ID, state);
    objects->device_obj.readFunc = read_device;
    objects->device_obj.writeFunc = write_device;
    objects->device_obj.executeFunc = execute_device;
    init_object(&objects->firmware_obj, &objects->firmware_inst, LWM2M_FIRMWARE_UPDATE_OBJECT_ID, state);
    objects->firmware_obj.readFunc = read_firmware;
    objects->firmware_obj.writeFunc = write_firmware;
    objects->firmware_obj.executeFunc = execute_firmware;
}

static int open_udp_connection(const char *host, uint16_t port, int *fd, lwm2m_uniproton_connection_t *connection)
{
    struct sockaddr_in addr;

    if (host == NULL || port == 0U || fd == NULL || connection == NULL) {
        return -1;
    }
    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*fd < 0) {
        return -1;
    }
    (void)memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1 || connect(*fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        (void)close(*fd);
        return -1;
    }
    (void)memset(connection, 0, sizeof(*connection));
    connection->fd = *fd;
    connection->server_ip = host;
    connection->server_port = port;
    connection->session.fd = *fd;
    connection->session.peer_len = sizeof(addr);
    (void)memcpy(&connection->session.peer, &addr, sizeof(addr));
    return 0;
}

static int run_lwm2m_loop(const char *host, uint16_t port, const char *endpoint, iot_lwm2m_core_state_s *state)
{
    lwm2m_uniproton_connection_t connection;
    iot_lwm2m_objects_s objects;
    lwm2m_object_t *object_array[4];
    lwm2m_context_t *context;
    uint8_t buf[768];
    time_t timeout;
    int object_count;
    int fd = -1;
    int len;
    int i;

    if (state == NULL || endpoint == NULL || open_udp_connection(host, port, &fd, &connection) != 0) {
        return -1;
    }
    init_objects(&objects, state);
    (void)snprintf(state->server_uri, sizeof(state->server_uri), "coap://%s:%u", host, port);
    object_array[0] = &objects.security_obj;
    object_array[1] = &objects.server_obj;
    object_array[2] = &objects.device_obj;
    object_count = 3;
    if (state->mode == IOT_LWM2M_MODE_OTA_PACKAGE) {
        object_array[object_count++] = &objects.firmware_obj;
    }

    context = lwm2m_init(&connection);
    if (context == NULL) {
        (void)close(fd);
        return -1;
    }
    if (lwm2m_configure(context, endpoint, NULL, NULL, (uint16_t)object_count, object_array) != COAP_NO_ERROR) {
        lwm2m_close(context);
        (void)close(fd);
        return -1;
    }

    (void)set_recv_timeout(fd, 1000U);
    for (i = 0; i < 60; i++) {
        timeout = 1;
        (void)lwm2m_step(context, &timeout);
        if ((state->mode == IOT_LWM2M_MODE_DEMO && state->read_ok && state->write_ok && state->execute_ok) ||
            (state->mode == IOT_LWM2M_MODE_OTA_PACKAGE && state->fw_executed && state->package_len > 0U)) {
            lwm2m_close(context);
            (void)close(fd);
            return 0;
        }
        len = (int)recv(fd, buf, sizeof(buf), 0);
        if (len > 0) {
            lwm2m_handle_packet(context, buf, len, &connection.session);
        }
    }

    lwm2m_close(context);
    (void)close(fd);
    return -1;
}

int iot_lwm2m_core_run_demo(const char *host, uint16_t port, const char *endpoint)
{
    iot_lwm2m_core_state_s state;

    (void)memset(&state, 0, sizeof(state));
    state.mode = IOT_LWM2M_MODE_DEMO;
    state.current_time = 1604394560LL;
    (void)snprintf(state.utc_offset, sizeof(state.utc_offset), "+01:00");
    (void)snprintf(state.timezone, sizeof(state.timezone), "Europe/Berlin");
    return run_lwm2m_loop(host, port, endpoint, &state);
}

int iot_lwm2m_core_receive_package(const char *host, uint16_t port, const char *endpoint,
    uint8_t *package, uint32_t package_capacity, uint32_t *package_len)
{
    iot_lwm2m_core_state_s state;
    int ret;

    if (package == NULL || package_capacity == 0U || package_len == NULL) {
        return -1;
    }
    (void)memset(&state, 0, sizeof(state));
    state.mode = IOT_LWM2M_MODE_OTA_PACKAGE;
    state.package = package;
    state.package_capacity = package_capacity;
    ret = run_lwm2m_loop(host, port, endpoint, &state);
    if (ret == 0) {
        *package_len = state.package_len;
    }
    return ret;
}

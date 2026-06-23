#include "lwm2m_uniproton_port.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "liblwm2m.h"
#include "connection.h"
#include "object_comm.h"
#include "prt_sem.h"
#include "prt_sys_external.h"
#include "prt_task.h"
#include "prt_tick.h"

static uint64_t lwm2m_now_ms(void)
{
    return (PRT_TickGetCount() * 1000ULL) / OsSysGetTickPerSecond();
}

void *lwm2m_malloc(size_t size)
{
    return malloc(size);
}

void lwm2m_free(void *ptr)
{
    free(ptr);
}

char *lwm2m_strdup(const char *str)
{
    size_t len;
    char *copy;

    if (str == NULL) {
        return NULL;
    }
    len = strlen(str) + 1;
    copy = (char *)lwm2m_malloc(len);
    if (copy == NULL) {
        return NULL;
    }
    (void)memcpy(copy, str, len);
    return copy;
}

int lwm2m_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

time_t lwm2m_gettime(void)
{
    return (time_t)(lwm2m_now_ms() / 1000ULL);
}

int lwm2m_rand(void *output, size_t len)
{
    uint8_t *bytes = (uint8_t *)output;
    uint32_t seed = (uint32_t)PRT_TickGetCount();
    size_t i;

    if (bytes == NULL) {
        return -1;
    }
    for (i = 0; i < len; i++) {
        seed = seed * 1103515245U + 12345U;
        bytes[i] = (uint8_t)(seed >> 16);
    }
    return 0;
}

void lwm2m_delay(uint32_t second)
{
    U64 ticks = (U64)second * OsSysGetTickPerSecond();

    if (ticks > 0xFFFFFFFFULL) {
        ticks = 0xFFFFFFFFULL;
    }
    (void)PRT_TaskDelay((U32)ticks);
}

uint64_t atiny_gettime_ms(void)
{
    return lwm2m_now_ms();
}

void atiny_usleep(unsigned long usec)
{
    U64 ticks = ((U64)usec * OsSysGetTickPerSecond() + 999999ULL) / 1000000ULL;

    if (ticks == 0 && usec != 0) {
        ticks = 1;
    }
    if (ticks > 0xFFFFFFFFULL) {
        ticks = 0xFFFFFFFFULL;
    }
    (void)PRT_TaskDelay((U32)ticks);
}

int atiny_random(void *output, size_t len)
{
    return lwm2m_rand(output, len);
}

void *atiny_malloc(size_t size)
{
    return lwm2m_malloc(size);
}

void atiny_free(void *ptr)
{
    lwm2m_free(ptr);
}

char *atiny_strdup(const char *str)
{
    return lwm2m_strdup(str);
}

void *atiny_mutex_create(void)
{
    SemHandle *handle = (SemHandle *)lwm2m_malloc(sizeof(SemHandle));

    if (handle == NULL) {
        return NULL;
    }
    if (PRT_SemCreate(1, handle) != OS_OK) {
        lwm2m_free(handle);
        return NULL;
    }
    return handle;
}

void atiny_mutex_destroy(void *mutex)
{
    if (mutex != NULL) {
        (void)PRT_SemDelete(*(SemHandle *)mutex);
        lwm2m_free(mutex);
    }
}

void atiny_mutex_lock(void *mutex)
{
    if (mutex != NULL) {
        (void)PRT_SemPend(*(SemHandle *)mutex, OS_WAIT_FOREVER);
    }
}

void atiny_mutex_unlock(void *mutex)
{
    if (mutex != NULL) {
        (void)PRT_SemPost(*(SemHandle *)mutex);
    }
}

void atiny_delay(uint32_t second)
{
    lwm2m_delay(second);
}

int atiny_printf(const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int atiny_snprintf(char *buf, unsigned int size, const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsnprintf(buf, size, format, args);
    va_end(args);
    return ret;
}

void lwm2m_register_connection_err_notify(lwm2m_connection_err_notify_t notify)
{
    (void)notify;
}

void *lwm2m_connect_server(uint16_t secObjInstID, void *userData, bool isServer)
{
    lwm2m_uniproton_connection_t *connection = (lwm2m_uniproton_connection_t *)userData;

    (void)secObjInstID;
    (void)isServer;
    if (connection == NULL || connection->fd < 0) {
        return NULL;
    }
    return &connection->session;
}

void lwm2m_close_connection(void *sessionH, void *userData)
{
    (void)sessionH;
    (void)userData;
}

uint8_t lwm2m_buffer_send(void *sessionH, uint8_t *buffer, size_t length, void *userData)
{
    lwm2m_uniproton_session_t *session = (lwm2m_uniproton_session_t *)sessionH;
    lwm2m_uniproton_connection_t *connection = (lwm2m_uniproton_connection_t *)userData;
    ssize_t ret;

    if (session == NULL || buffer == NULL || connection == NULL || connection->fd < 0) {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
    ret = sendto(connection->fd, buffer, length, 0, (struct sockaddr *)&session->peer, session->peer_len);
    return (ret == (ssize_t)length) ? COAP_NO_ERROR : COAP_500_INTERNAL_SERVER_ERROR;
}

int lwm2m_buffer_recv(void *sessionH, uint8_t *buffer, size_t length, uint32_t timeout)
{
    lwm2m_uniproton_session_t *session = (lwm2m_uniproton_session_t *)sessionH;
    struct timeval tv;
    ssize_t ret;

    if (session == NULL || buffer == NULL || session->fd < 0) {
        return -1;
    }
    tv.tv_sec = timeout / 1000U;
    tv.tv_usec = (timeout % 1000U) * 1000U;
    (void)setsockopt(session->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    session->peer_len = sizeof(session->peer);
    ret = recvfrom(session->fd, buffer, length, 0, (struct sockaddr *)&session->peer, &session->peer_len);
    return (ret < 0) ? -1 : (int)ret;
}

bool lwm2m_session_is_equal(void *session1, void *session2, void *userData)
{
    (void)userData;
    return session1 == session2;
}

uint8_t acc_auth_operate(lwm2m_context_t *contextP, lwm2m_uri_t *uri, OBJ_ACC_OPERATE op, uint16_t serverId)
{
    (void)contextP;
    (void)uri;
    (void)op;
    (void)serverId;
    return COAP_NO_ERROR;
}

void output_buffer(FILE *stream, uint8_t *buffer, int length, int indent)
{
    int i;

    (void)indent;
    if (stream == NULL || buffer == NULL || length <= 0) {
        return;
    }
    for (i = 0; i < length; i++) {
        (void)fprintf(stream, "%02X", buffer[i]);
    }
    (void)fprintf(stream, "\n");
}

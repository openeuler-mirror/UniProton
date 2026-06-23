#ifndef LWM2M_UNIPROTON_PORT_H
#define LWM2M_UNIPROTON_PORT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int fd;
    struct sockaddr_storage peer;
    socklen_t peer_len;
} lwm2m_uniproton_session_t;

typedef struct {
    int fd;
    const char *server_ip;
    uint16_t server_port;
    lwm2m_uniproton_session_t session;
} lwm2m_uniproton_connection_t;

void *lwm2m_malloc(size_t size);
void lwm2m_free(void *ptr);
char *lwm2m_strdup(const char *str);
int lwm2m_strncmp(const char *s1, const char *s2, size_t n);
time_t lwm2m_gettime(void);
int lwm2m_rand(void *output, size_t len);
void lwm2m_delay(uint32_t second);

uint64_t atiny_gettime_ms(void);
void atiny_usleep(unsigned long usec);
int atiny_random(void *output, size_t len);
void *atiny_malloc(size_t size);
void atiny_free(void *ptr);
char *atiny_strdup(const char *str);
void *atiny_mutex_create(void);
void atiny_mutex_destroy(void *mutex);
void atiny_mutex_lock(void *mutex);
void atiny_mutex_unlock(void *mutex);
void atiny_delay(uint32_t second);
int atiny_printf(const char *format, ...);
int atiny_snprintf(char *buf, unsigned int size, const char *format, ...);

int lwm2m_buffer_recv(void *sessionH, uint8_t *buffer, size_t length, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif

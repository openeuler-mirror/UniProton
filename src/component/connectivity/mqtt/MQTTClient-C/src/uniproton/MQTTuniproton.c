/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: UniProton MQTT client platform adapter for sockets.
 * --------------------------------------------------------------------------- */

#include "MQTTuniproton.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "prt_sys_external.h"
#include "prt_tick.h"

static unsigned long long mqtt_now_ms(void)
{
    return (PRT_TickGetCount() * 1000ULL) / OsSysGetTickPerSecond();
}

void TimerInit(Timer *timer)
{
    timer->end_time = mqtt_now_ms();
}

char TimerIsExpired(Timer *timer)
{
    return mqtt_now_ms() >= timer->end_time;
}

void TimerCountdownMS(Timer *timer, unsigned int timeout)
{
    timer->end_time = mqtt_now_ms() + timeout;
}

void TimerCountdown(Timer *timer, unsigned int timeout)
{
    TimerCountdownMS(timer, timeout * 1000U);
}

int TimerLeftMS(Timer *timer)
{
    unsigned long long now = mqtt_now_ms();

    return (timer->end_time <= now) ? 0 : (int)(timer->end_time - now);
}

static int mqtt_set_recv_timeout(int fd, int timeout_ms)
{
    struct timeval tv;

    if (timeout_ms < 0) {
        timeout_ms = 0;
    }
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int mqtt_read(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
    int done = 0;

    if (n == NULL || buffer == NULL || len <= 0 || n->fd < 0) {
        return -1;
    }
    (void)mqtt_set_recv_timeout(n->fd, timeout_ms);
    while (done < len) {
        int ret = (int)recv(n->fd, &buffer[done], (size_t)(len - done), 0);
        if (ret == 0) {
            return 0;
        }
        if (ret < 0) {
            return (done == 0) ? 0 : done;
        }
        done += ret;
    }
    return done;
}

static int mqtt_write(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
    int done = 0;

    (void)timeout_ms;
    if (n == NULL || buffer == NULL || len <= 0 || n->fd < 0) {
        return -1;
    }
    while (done < len) {
        int ret = (int)send(n->fd, &buffer[done], (size_t)(len - done), 0);
        if (ret <= 0) {
            return -1;
        }
        done += ret;
    }
    return done;
}

void NetworkInit(Network *n)
{
    if (n == NULL) {
        return;
    }
    (void)memset(n, 0, sizeof(Network));
    n->fd = -1;
    n->mqttread = mqtt_read;
    n->mqttwrite = mqtt_write;
}

int NetworkConnect(Network *n, char *addr, int port)
{
    struct sockaddr_in server;
    int fd;

    if (n == NULL || addr == NULL || port <= 0) {
        return -1;
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    (void)memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, addr, &server.sin_addr) != 1 || connect(fd, (struct sockaddr *)&server, sizeof(server)) != 0) {
        (void)close(fd);
        return -1;
    }
    n->fd = fd;
    return 0;
}

void NetworkDisconnect(Network *n)
{
    if (n != NULL && n->fd >= 0) {
        (void)close(n->fd);
        n->fd = -1;
    }
}

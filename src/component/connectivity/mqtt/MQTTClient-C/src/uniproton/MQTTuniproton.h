/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: UniProton MQTT client platform definitions.
 * --------------------------------------------------------------------------- */

#ifndef MQTT_UNIPROTON_H
#define MQTT_UNIPROTON_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Timer {
    unsigned long long end_time;
} Timer;

void TimerInit(Timer *timer);
char TimerIsExpired(Timer *timer);
void TimerCountdownMS(Timer *timer, unsigned int timeout);
void TimerCountdown(Timer *timer, unsigned int timeout);
int TimerLeftMS(Timer *timer);

typedef struct Network {
    int fd;
    int (*mqttread)(struct Network *n, unsigned char *buffer, int len, int timeout_ms);
    int (*mqttwrite)(struct Network *n, unsigned char *buffer, int len, int timeout_ms);
} Network;

void NetworkInit(Network *n);
int NetworkConnect(Network *n, char *addr, int port);
void NetworkDisconnect(Network *n);

#ifdef __cplusplus
}
#endif

#endif

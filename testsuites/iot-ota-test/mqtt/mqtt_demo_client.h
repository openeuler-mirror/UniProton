#ifndef UNIPROTON_CONNECTIVITY_MQTT_DEMO_CLIENT_H
#define UNIPROTON_CONNECTIVITY_MQTT_DEMO_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int mqtt_demo_run(const char *host, uint16_t port, const char *client_id);

#ifdef __cplusplus
}
#endif

#endif

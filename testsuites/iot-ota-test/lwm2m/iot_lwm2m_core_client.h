#ifndef IOT_LWM2M_CORE_CLIENT_H
#define IOT_LWM2M_CORE_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int iot_lwm2m_core_run_demo(const char *host, uint16_t port, const char *endpoint);
int iot_lwm2m_core_receive_package(const char *host, uint16_t port, const char *endpoint,
    uint8_t *package, uint32_t package_capacity, uint32_t *package_len);

#ifdef __cplusplus
}
#endif

#endif

#ifndef UNIPROTON_CONNECTIVITY_LWM2M_OTA_CLIENT_H
#define UNIPROTON_CONNECTIVITY_LWM2M_OTA_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int lwm2m_ota_register_and_receive_package(const char *host, uint16_t port, const char *endpoint,
    uint8_t *package, uint32_t package_capacity, uint32_t *package_len);

#ifdef __cplusplus
}
#endif

#endif

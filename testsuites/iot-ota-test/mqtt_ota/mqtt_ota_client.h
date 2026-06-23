#ifndef UNIPROTON_CONNECTIVITY_MQTT_OTA_CLIENT_H
#define UNIPROTON_CONNECTIVITY_MQTT_OTA_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int mqtt_ota_fetch_package(const char *host, uint16_t port, const char *client_id,
    uint8_t *package, uint32_t package_len, uint32_t chunk_size);
int mqtt_ota_fetch_manifest_and_package(const char *host, uint16_t port, const char *client_id,
    uint8_t *package, uint32_t package_capacity, uint32_t chunk_size, uint32_t *package_len);

#ifdef __cplusplus
}
#endif

#endif

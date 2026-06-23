#include "lwm2m_ota_client.h"

#include "iot_lwm2m_core_client.h"

int lwm2m_ota_register_and_receive_package(const char *host, uint16_t port, const char *endpoint,
    uint8_t *package, uint32_t package_capacity, uint32_t *package_len)
{
    return iot_lwm2m_core_receive_package(host, port, endpoint, package, package_capacity, package_len);
}

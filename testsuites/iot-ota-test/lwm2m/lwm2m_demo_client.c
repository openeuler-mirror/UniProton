#include "lwm2m_demo_client.h"

#include "iot_lwm2m_core_client.h"

int lwm2m_demo_register_and_process(const char *host, uint16_t port, const char *endpoint)
{
    return iot_lwm2m_core_run_demo(host, port, endpoint);
}

#ifndef UNIPROTON_CONNECTIVITY_LWM2M_DEMO_CLIENT_H
#define UNIPROTON_CONNECTIVITY_LWM2M_DEMO_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int lwm2m_demo_register_and_process(const char *host, uint16_t port, const char *endpoint);

#ifdef __cplusplus
}
#endif

#endif

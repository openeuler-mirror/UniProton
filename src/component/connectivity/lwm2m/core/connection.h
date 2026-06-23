/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: UniProton LwM2M UDP connection adapter interface.
 * --------------------------------------------------------------------------- */

#ifndef LWM2M_UNIPROTON_CONNECTION_H
#define LWM2M_UNIPROTON_CONNECTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwm2m_uniproton_port.h"

#define LWM2M_IS_CLIENT 0
#define LWM2M_IS_SERVER 1

typedef enum {
    CONNECTION_SEND_ERR,
    CONNECTION_RECV_ERR,
    CONNECTION_ERR_MAX
} connection_err_e;

typedef lwm2m_uniproton_session_t connection_t;

typedef void (*lwm2m_connection_err_notify_t)(lwm2m_context_t *context, connection_err_e err_type, bool bootstrap_flag);

int lwm2m_buffer_recv(void *sessionH, uint8_t *buffer, size_t length, uint32_t timeout);
void lwm2m_register_connection_err_notify(lwm2m_connection_err_notify_t notify);

#endif

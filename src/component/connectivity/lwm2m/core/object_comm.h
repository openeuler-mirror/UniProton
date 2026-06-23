/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: Minimal object common definitions for UniProton LwM2M core.
 * --------------------------------------------------------------------------- */

#ifndef LWM2M_UNIPROTON_OBJECT_COMM_H
#define LWM2M_UNIPROTON_OBJECT_COMM_H

#include "internals.h"

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

typedef enum {
    OBJ_ACC_READ = 0,
    OBJ_ACC_WRITE_ATTR = 0,
    OBJ_ACC_OBSERVE = 0,
    OBJ_ACC_NOTIFY = 0,
    OBJ_ACC_WRITE = 1,
    OBJ_ACC_EXCUTE = 2,
    OBJ_ACC_DELETE = 3,
    OBJ_ACC_CREATE = 4,
    OBJ_ACC_DISCOVER = 5,
} OBJ_ACC_OPERATE;

uint8_t acc_auth_operate(lwm2m_context_t *contextP, lwm2m_uri_t *uri, OBJ_ACC_OPERATE op, uint16_t serverId);

#endif

/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-25
 * Description: shell los_mux 适配头文件。
 */
#ifndef _LOS_MUX_H
#define _LOS_MUX_H

#include "pthread.h"
#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOS_WAIT_FOREVER
#define LOS_WAIT_FOREVER OS_EVENT_WAIT_FOREVER
#endif

extern UINT32 LOS_MuxCreate(pthread_mutex_t *muxHandle);

extern UINT32 LOS_MuxPend(pthread_mutex_t *muxHandle, UINT32 timeout);

extern UINT32 LOS_MuxPost(pthread_mutex_t *muxHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MUX_H */
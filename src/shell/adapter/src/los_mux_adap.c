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
 * Description: shell los mutex 适配实现。
 */
#include "pthread.h"
#include "los_mux.h"

UINT32 LOS_MuxCreate(pthread_mutex_t *muxHandle)
{
    return pthread_mutex_init(muxHandle, NULL);
}

UINT32 LOS_MuxPend(pthread_mutex_t *muxHandle, UINT32 timeout)
{
    (void)timeout;
    return pthread_mutex_lock(muxHandle);
}

UINT32 LOS_MuxPost(pthread_mutex_t *muxHandle)
{
    return pthread_mutex_unlock(muxHandle);
}
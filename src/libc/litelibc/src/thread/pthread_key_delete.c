/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread_key_delete 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int __pthread_key_delete(pthread_key_t key)
{
    U32 i;
    uintptr_t intSave;

    if ((U32)key >= PTHREAD_KEYS_MAX || g_pthread_keys_destor[key] == NULL) {
        return EINVAL;
    }

    intSave = OsIntLock();
    for (i = 0; i < g_tskMaxNum; i++) {
        g_tskCbArray[i].tsdUsed = g_tskCbArray[i].tsdUsed & ~(1U << (U32)key);
        g_tskCbArray[i].tsd[key] = NULL;
    }
    g_pthread_keys_destor[key] = NULL;
    OsIntRestore(intSave);

    return OS_OK;
}

weak_alias(__pthread_key_delete, pthread_key_delete);
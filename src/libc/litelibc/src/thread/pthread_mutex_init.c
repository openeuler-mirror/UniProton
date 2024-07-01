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
 * Description: pthread_mutex_init 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

/* pthread mutex attr中的proctol属性无效，是否开启优先级继承由宏控制 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    U32 ret;
    U32 protocol;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (attr == NULL) {
        mutex->type = PTHREAD_MUTEX_DEFAULT;
        protocol = PTHREAD_PRIO_NONE;
    } else {
        mutex->type = (U8)attr->type;
        protocol = (U32)attr->protocol;
    }

    switch (mutex->type) {
        case PTHREAD_MUTEX_NORMAL:
        case PTHREAD_MUTEX_ERRORCHECK:
            ret = OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (protocol << 8), SEM_MODE_PRIOR,
                              (SemHandle *)&mutex->mutex_sem, (U32)&mutex->mutex_sem);
            break;

        case PTHREAD_MUTEX_RECURSIVE:
            ret = OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (SEM_MUTEX_TYPE_RECUR << 4) | (protocol << 8), SEM_MODE_PRIOR,
                              (SemHandle *)&mutex->mutex_sem, (U32)&mutex->mutex_sem);
            break;

        default:
            ret = EINVAL;
            break;
    }

    if (ret != OS_OK) {
        return EINVAL;
    }
    mutex->magic = MUTEX_MAGIC;

    return OS_OK;
}
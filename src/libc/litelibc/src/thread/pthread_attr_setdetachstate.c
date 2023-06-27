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
 * Description: pthread_attr_setdetachstate 接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachState)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (detachState != PTHREAD_CREATE_JOINABLE && detachState != PTHREAD_CREATE_DETACHED) {
        return EINVAL;
    }

    attr->detachstate = (unsigned int)detachState;

    return OS_OK;
}
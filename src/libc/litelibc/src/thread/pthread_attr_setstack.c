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
 * Description: pthread_attr_setstack 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_attr_setstack(pthread_attr_t *attr, void *stackAddr, size_t stackSize)
{
    U64 stackAddrLen;

    if (attr == NULL) {
        return EINVAL;
    }

    if (stackSize < (long)OS_TSK_MIN_STACK_SIZE) {
        return EINVAL;
    }

    stackAddrLen = (U64)(stackSize);
    /* posix接口不实现msgq, queNum默认为0 */
    if ((uintptr_t)stackAddr != 0U) {
        stackAddrLen += (uintptr_t)(stackAddr);
    }
    /* 保证栈空间在4G范围内不溢出 */
    if (stackAddrLen > OS_MAX_U32) {
        return EINVAL;
    }

    attr->stackaddr = stackAddr;
    attr->stackaddr_set = 1;
    attr->stacksize = stackSize;
    attr->stacksize_set = 1;

    return OS_OK;
}
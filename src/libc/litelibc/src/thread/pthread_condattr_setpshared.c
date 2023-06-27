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
 * Create: 2023-06-01
 * Description: pthread_condattr_setpshared 相关接口实现
 */
#include <pthread.h>
#include "prt_posix_internal.h"

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
    (void)attr;
    if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED) {
        return EINVAL;
    }

    if (pshared != PTHREAD_PROCESS_PRIVATE) {
        return ENOTSUP;
    }
    return 0;
}
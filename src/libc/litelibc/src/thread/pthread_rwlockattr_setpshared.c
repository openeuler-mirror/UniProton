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
 * Description: pthread_rwlockattr_setpshared 相关接口实现，暂不支持该接口
 */
#include <pthread.h>
#include <errno.h>

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
	// UniProton无进程，仅支持PTHREAD_PROCESS_PRIVATE
    (void)attr;
    if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED) {
        return EINVAL;
    }

    if (pshared != PTHREAD_PROCESS_PRIVATE) {
        return ENOTSUP;
    }
    return 0;
}
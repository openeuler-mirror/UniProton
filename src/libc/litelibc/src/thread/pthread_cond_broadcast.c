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
 * Description: pthread_cond_broadcast 相关接口实现
 */
#include <pthread.h>
#include "prt_posix_internal.h"

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    if (OsCondParamCheck(cond) != OS_OK) {
        return EINVAL;
    }
    return __private_cond_signal(cond, 0);
}
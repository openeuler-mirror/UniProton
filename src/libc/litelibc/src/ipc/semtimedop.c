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
 * Create: 2023-08-15
 * Description: semtimedop实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/sem.h>
#include "prt_posix_internal.h"
#include "prt_ipc_internal.h"

int semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout)
{
    if (timeout == NULL) {
        errno = EINVAL;
        return -1;
    }
    errno = OsSemSetTimeOp(semid, sops, nsops, timeout);
    return (errno == 0) ? 0 : (-1);
}
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
 * Description: shmat实现
 */
#include <errno.h>
#include <sys/shm.h>
#include "prt_ipc_internal.h"

void *shmat(int shmid, const void *shmaddr, int shmflg)
{
    void *addr;
    int ret = OsShmAt(shmid, shmaddr, shmflg, &addr);
    errno = ret;
    return (ret == 0) ? addr : ((void *)-1);
}
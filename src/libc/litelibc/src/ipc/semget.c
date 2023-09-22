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
 * Description: semget实现
 */
#include <sys/sem.h>
#include "kal_ipc.h"
#include "prt_ipc_internal.h"

int semget(key_t key, int nsems, int flag)
{
    int semid, mode;
    mode = flag & IPC_DEFAULT_MODE_MASK;
    errno = OsSemSetGet(key, nsems, flag, mode, &semid);
    return (errno == 0) ? semid : (-1);
}
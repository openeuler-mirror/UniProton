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
 * Description: semctl实现
 */
#include <sys/sem.h>
#include "kal_ipc.h"
#include "prt_ipc_internal.h"

int semctl(int semid, int semnum, int cmd, ...)
{
    int value;
    union semun arg = {0};
    if (cmd == IPC_RMID) {
        errno = OsSemSetDelete(semid);
        return (errno == 0) ? 0 : (-1);
    }
    if (cmd == GETVAL) {
        errno = OsSemSetGetVal(semid, semnum, &value);
        return (errno == 0) ? value : (-1);
    }
    va_list ap;
    va_start(ap, cmd);
    arg = va_arg(ap, union semun);
    va_end(ap);
    switch (cmd) {
        case IPC_STAT:
            errno = OsSemSetStat(semid, arg.buf);
            break;
        case GETALL:
            errno = OsSemSetGetAll(semid, arg.array);
            break;
        default:
            break;
    }
    return (errno == 0) ? 0 : (-1);
}
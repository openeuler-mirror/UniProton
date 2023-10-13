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
 * Description: shmctl实现
 */
#include <errno.h>
#include <sys/shm.h>
#include "prt_ipc_internal.h"

int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    int ret;
    switch (cmd) {
        case IPC_RMID:
            ret = OsShmDelete(shmid);
            break;
        case IPC_SET:
            ret = 0;
            break;
        case IPC_STAT:
            ret = OsShmGetStat(shmid, buf);
            break;
        case IPC_INFO:
            ret = OsShmGetIpcInfo((struct shminfo *)buf);
            if (ret == 0) {
                return SHMSEG_MAX_SHM_LIMIT;
            }
            break;
        case SHM_INFO:
            ret = OsShmGetShmInfo((struct shm_info *)buf);
            break;
        case SHM_STAT:
        case SHM_STAT_ANY:
            ret = OsShmGetStat(OS_IPC_ID(shmid), buf);
            if (ret == 0) {
                return OS_IPC_ID(shmid);
            }
            break;
        case SHM_LOCK:
        case SHM_UNLOCK:
            ret = 0;
            break;
        default:
            ret = EINVAL;
    }
    errno = ret;
    return (ret == 0) ? 0 : (-1);
}
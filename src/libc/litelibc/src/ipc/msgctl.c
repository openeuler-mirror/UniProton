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
 * Description: msgctl实现
 */
#include <sys/msg.h>
#include "kal_ipc.h"
#include "prt_ipc_internal.h"
#include "prt_queue_external.h"

int msgctl(int msgid, int cmd, struct msqid_ds *buf)
{
    uintptr_t intSave;
    switch (cmd) {
        case IPC_STAT:
            errno = KAL_MsgCtlStat((uintptr_t)msgid, buf);
            break;
        case IPC_RMID:
            intSave = OsIntLock();
            errno = KAL_MsgDelete((uintptr_t)msgid);
            if (errno == 0) {
                OsMsgQueDelById(msgid);
            }
            OsIntRestore(intSave);
            break;
        default:
            errno = ENOTSUP;
            break;
    }
    return (errno == 0) ? 0 : (-1);
}
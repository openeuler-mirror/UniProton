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
 * Description: msgget实现
 */
#include <sys/msg.h>
#include <stdio.h>
#include "kal_ipc.h"
#include "prt_ipc_internal.h"
#include "prt_queue_external.h"

int msgget(key_t key, int flag)
{
    uintptr_t intSave;
    int msgid, mode;
    mode = flag & IPC_DEFAULT_MODE_MASK;
    if (key == IPC_PRIVATE) {
        errno = KAL_MsgCreate(flag, mode, MSGQUE_MAX_MSG_NUM, MSGQUE_MAX_MSG_SIZE, &msgid);
        return (errno == 0) ? msgid : (-1);
    }
    intSave = OsIntLock();
    msgid = OsMsgQueKey2Id(key);
    errno = KAL_MsgFind(msgid);
    if (errno == 0) {
        if (flag & (IPC_CREAT | IPC_EXCL)) {
            errno = EEXIST;
        }
    } else if (flag & IPC_CREAT) {
        errno = KAL_MsgCreate(flag, mode, MSGQUE_MAX_MSG_NUM, MSGQUE_MAX_MSG_SIZE, &msgid);
        if (errno == 0) {
            OsMsgQueAddKeyId(key, msgid);
        }
    }
    OsIntRestore(intSave);
    return (errno == 0) ? msgid : (-1);
}
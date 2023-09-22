/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-10
 * Description: os内部ipc msg功能实现
 */
#include "securec.h"
#include "prt_mem_external.h"
#include "prt_queue_external.h"
#include "prt_ipc_internal.h"

static struct MsgQueCb g_ipcMsgQue[MSGQUE_MAX_SYS_LIMIT];

void OsMsgQueAddKeyId(key_t key, int msgid)
{
    int idx;
    for (idx = 0; idx < MSGQUE_MAX_SYS_LIMIT; ++idx) {
        if (g_ipcMsgQue[idx].msgid == 0) {
            g_ipcMsgQue[idx].key = key;
            g_ipcMsgQue[idx].msgid = msgid;
            return;
        }
    }
}

void OsMsgQueDelById(int msgid)
{
    int idx;
    for (idx = 0; idx < MSGQUE_MAX_SYS_LIMIT; ++idx) {
        if (g_ipcMsgQue[idx].msgid == msgid) {
            g_ipcMsgQue[idx].msgid = 0;
            g_ipcMsgQue[idx].key = 0;
            return;
        }
    }
}

int OsMsgQueKey2Id(key_t key)
{
    int idx;
    for (idx = 0; idx < MSGQUE_MAX_SYS_LIMIT; ++idx) {
        if (g_ipcMsgQue[idx].msgid > 0 && g_ipcMsgQue[idx].key == key) {
            return g_ipcMsgQue[idx].msgid;
        }
    }
    return -1;
}

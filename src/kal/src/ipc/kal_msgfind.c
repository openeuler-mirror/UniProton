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
 * Create: 2023-08-12
 * Description: KAL msg find实现。
 */
#include <errno.h>
#include "kal_ipc.h"
#include "prt_queue_external.h"

int KAL_MsgFind(uintptr_t msgid)
{
    uintptr_t intSave;
    struct TagQueCb *queueCb = NULL;
    U32 innerId = OS_QUEUE_INNER_ID(msgid);
    if (innerId >= g_maxQueue) {
        return ENOENT;
    }
    intSave = OsIntLock();
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    if (queueCb && queueCb->queueState == OS_QUEUE_USED) {
        OsIntRestore(intSave);
        return 0;
    }
    OsIntRestore(intSave);
    return ENOENT;
}
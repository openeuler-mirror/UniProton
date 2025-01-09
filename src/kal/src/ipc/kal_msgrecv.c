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
 * Description: KAL msg recv实现。
 */
#include <errno.h>
#include "kal_ipc.h"
#include "prt_queue_external.h"

int KAL_MsgRecv(uintptr_t msqid, void *msgp, ssize_t *msgsz, long msgtype, int flag, const struct timespec *ts)
{
    U32 ret, timeout;
    (void)ts;
    if (msgtype != 0) {
        return ENOTSUP;
    }
    timeout = (flag & IPC_NOWAIT) ? OS_QUEUE_NO_WAIT : OS_QUEUE_WAIT_FOREVER;
    ret = PRT_QueueRead((U32)msqid, msgp, (U32 *)msgsz, timeout);
    switch (ret) {
        case OS_OK:
            return 0;
        case OS_ERRNO_QUEUE_NOT_CREATE:
            return EIDRM;
        case OS_ERRNO_QUEUE_NO_SOURCE:
            return ENOMSG;
        default:
            break;
    }
    return EINVAL;
}
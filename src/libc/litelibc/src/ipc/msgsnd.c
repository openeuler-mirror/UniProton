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
 * Description: msgsnd实现
 */
#include <sys/msg.h>
#include "kal_ipc.h"
#include "prt_lib_external.h"
#include "prt_queue_external.h"

int msgsnd(int msqid, const void *msgp, size_t msgsz, int flag)
{
    struct msgbuf *buf;
    if (msgp == NULL) {
        errno = EFAULT;
        return -1;
    }
    buf = (struct msgbuf *)msgp;
    if (msqid < 0 || msgsz > OS_MAX_U32 || buf->mtype < 1) {
        errno = EINVAL;
        return -1;
    }
    errno = KAL_MsgSend(msqid, msgp, msgsz, flag, NULL);
    return (errno == 0) ? 0 : (-1);
}
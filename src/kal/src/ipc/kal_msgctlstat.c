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
 * Description: KAL msg ctrl stat实现。
 */
#include <errno.h>
#include "prt_queue_external.h"
#include "kal_ipc.h"

int KAL_MsgCtlStat(uintptr_t msgid, struct msqid_ds *buf)
{
    uintptr_t intSave;
    int ret;
    U32 qnum, innerId;
    struct TagQueCb *queueCb = NULL;
    if (buf == NULL) {
        return EFAULT;
    }
    intSave = OsIntLock();
    ret = PRT_QueueGetNodeNum((U32)msgid, OS_QUEUE_PID_ALL, &qnum);
    if (ret != 0) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    innerId = OS_QUEUE_INNER_ID((U32)msgid);
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    buf->msg_qbytes = (msglen_t)queueCb->nodeNum * (msglen_t)queueCb->nodeSize;
    OsIntRestore(intSave);
    buf->msg_qnum = (msgqnum_t)qnum;
    return 0;
}
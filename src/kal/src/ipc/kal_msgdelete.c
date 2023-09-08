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
 * Description: KAL msg delete实现。
 */
#include <errno.h>
#include "prt_queue_external.h"
#include "kal_ipc.h"

int KAL_MsgDelete(uintptr_t msgid)
{
    U32 ret = PRT_QueueDelete((U32)msgid);
    switch (ret) {
        case OS_OK:
            return 0;
        case OS_ERRNO_QUEUE_NOT_CREATE:
            return EIDRM;
        default:
            break;
    }
    return EINVAL;
}
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
 * Description: KAL sem create实现。
 */
#include <errno.h>
#include "kal_ipc.h"
#include "prt_sem_external.h"

int KAL_SemCreate(int flag, int mode, int value, uintptr_t *semid)
{
    (void)flag;
    (void)mode;
    U32 ret = PRT_SemCreate((U32)value, (SemHandle *)semid);
    switch (ret) {
        case OS_OK:
            return 0;
        case OS_ERRNO_SEM_ALL_BUSY:
            return ENOSPC;
        default:
            break;
    }
    return EINVAL;
}
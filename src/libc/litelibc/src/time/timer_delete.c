/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: timer_delete 相关接口实现
 */
#include <time.h>
#include <errno.h>
#include "prt_timer.h"
#include "prt_posix_internal.h"

int timer_delete(timer_t timerId)
{
    U32 swtmrId = (U32)timerId;

    if (PRT_TimerDelete(0, swtmrId) != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    return OS_OK;
}
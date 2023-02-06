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
 * Create: 2022-11-15
 * Description: pthread cancel功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_cancel(pthread_t thread)
{
    return PRT_PthreadCancel((TskHandle)thread);
}

void pthread_testcancel(void)
{
    PRT_PthreadTestCancel();
}

int pthread_setcancelstate(int state, int *oldstate)
{
    return PRT_PthreadSetCancelState(state, oldstate);
}

int pthread_setcanceltype(int type, int *oldType)
{
    return PRT_PthreadSetCancelType(type, oldType);
}

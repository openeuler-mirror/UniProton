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
 * Create: 2023-06-08
 * Description: posix sigwaitinfo功能实现
 */
#include "signal.h"
#include "prt_signal.h"
#include "prt_posix_internal.h"

int sigwaitinfo(const sigset_t *__restrict set, siginfo_t *__restrict info)
{
    if (set == NULL || info == NULL) {
        return -1;
    }

    signalInfo prtInfo = {0};
    signalSet prtSet = set->__bits[0];
    U32 ret = PRT_SignalWait(&prtSet, &prtInfo, OS_SIGNAL_WAIT_FOREVER);
    if (ret != OS_OK) {
        return -1;
    }

    info->si_signo = prtInfo.si_signo;
    info->si_code = prtInfo.si_code;

    return 0;
}
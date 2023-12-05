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
 * Create: 2023-12-06
 * Description: posix signal功能实现
 */

#include "signal.h"

void (*signal(int sig, void (*func)(int)))(int)
{
    struct sigaction sa_old, sa = { .sa_handler = func, .sa_flags = SA_RESTART };
    if (sigaction(sig, &sa, &sa_old) < 0)
        return SIG_ERR;
    return sa_old.sa_handler;
}

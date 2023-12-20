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
 * Create: 2023-08-25
 * Description: shell uart 适配实现。
 */
#include <unistd.h>
#include "uart.h"
#include "prt_task_external.h"
#include "nuttx/arch.h"
#include "nuttx/sys/sys_unistd.h"

ssize_t UartRead(int fd, void *str, ssize_t n)
{
    return sys_read(fd, str, n);
}

VOID UartPuts(const CHAR *s, UINT32 len, BOOL isLock)
{
    UINT32 i;
    for (i = 0; i < len; i++) {
        if (*(s + i) == '\n') {
            up_putc('\r');
        }
        up_putc(*(s + i));
    }
}

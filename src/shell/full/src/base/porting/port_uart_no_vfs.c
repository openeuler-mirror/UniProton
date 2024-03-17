/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-03-03
 * Description: 没有使用VFS组件时,shell接收线程依赖的标准输入函数。
 */
 
#include "los_base.h"
#include "shell_pri.h"
#include "shmsg.h"
#include "console.h"
#include "uart.h"

UINT32 ShellStdinLoop(ShellCB *shellCB)
{
    UINT8 ch[1];
    ssize_t n;
    int fd = 1;
    while (1) {
        n = UartRead(fd, ch, sizeof(ch));
        if (n > 0) {
            if (ch[0] == '\n') {
                ch[0] = '\r';
            }
            ShellCmdLineParse(ch[0], (pf_OUTPUT)shprintf, shellCB);
        }
    }
    return 0;
}

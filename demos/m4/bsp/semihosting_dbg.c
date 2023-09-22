/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-7-5
 * Description: semihosting dbg 源文件
 */
#include "semihosting_dbg.h"
#include "prt_typedef.h"

#define ANGEL_SWI_INSN "bkpt"
#define ANGEL_SWI      0xAB

INLINE U32 CallHost(int reason, void *arg)
{
    int value;
    asm volatile (
        " mov r0, %[rsn] \n"
        " mov r1, %[arg] \n"
        " " ANGEL_SWI_INSN " %[swi] \n"
        " mov %[val], r0"
        : [val] "=r" (value)
        : [rsn] "r" (reason), [arg] "r" (arg), [swi] "i" (ANGEL_SWI)
        : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
    );

    return value;
}

/* 描述：semihosting dbg 打印函数 */
int SemihostingDbgWrite(const char *buf, int nbytes)
{
    if (buf[nbytes] == '\0') {
        CallHost(0x4, (void *)buf);
    } else {
        char tmp[16];
        int writeBytes = nbytes;
        while (writeBytes > 0) {
            int n = ((writeBytes < (int)(sizeof(tmp) - 1)) ? writeBytes : (int)(sizeof(tmp) - 1));
            int i = 0;
            for (; i < n; i++, buf++) {
                tmp[i] = *buf;
            }
            tmp[i] = '\0';

            CallHost(0x4, (void *)tmp);

            writeBytes -= n;
        }
    }

    return nbytes;
}

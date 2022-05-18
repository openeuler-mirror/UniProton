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
 * Create: 2009-07-24
 * Description: Hardware Initialization
 */
#include "prt_cpu_external.h"

#define OS_CPSR_REG_DEFAULT_VAL     0x01000000U
#define OS_EXCRETURN_DEFAULT_VAL    0xFFFFFFFDU

OS_SEC_L4_TEXT void *OsTskContextInit(U32 taskId, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    uintptr_t endStack;
    struct TagHwContext *context = NULL;

    /*
     * O0优化情况下，子函数会自动将父函数的栈指针，压入到父函数栈顶+4
     * 处，根函数没有父函数，需要构造虚拟父函数，预留16字节空间,
     * 未使用的空间作为维测使用
     * O2优化情况下，编译器生成指令不会进行上述操作，每个任务的栈底预留16
     * 个字节均初始化魔术字，作为维测使用，以判断是否有任务从栈底溢出
     */
    endStack = TRUNCATE((uintptr_t)topStack + stackSize, OS_TSK_STACK_ADDR_ALIGN);
    context = (struct TagHwContext *)(endStack - sizeof(struct TagHwContext));
    /* 初始化任务上下文 */
    *context = (struct TagHwContext){0};
    // 配置寄存器默认初值
    context->basePri = 0;
    context->excReturn = OS_EXCRETURN_DEFAULT_VAL;
    /* 任务的第一个参数 */
    context->r0 = taskId;
    context->lr = 0;
    context->pc = (uintptr_t)funcTskEntry;
    context->psr = OS_CPSR_REG_DEFAULT_VAL;           /* bit20表示thumb指令 */

    return (void *)context;
}

OS_SEC_L4_TEXT void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context)
{
    *context = *((struct TskContext *)saveAddr);
}

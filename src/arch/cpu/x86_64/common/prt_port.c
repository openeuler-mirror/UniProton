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
 * Create: 2023-06-25
 * Description: Hardware Initialization
 */
#include "prt_cpu_external.h"
#include "prt_sys_external.h"
#include "securec.h"

OS_SEC_L4_TEXT void *OsTskContextInit(U32 taskID, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    uintptr_t *stack;
    struct TagOsStack *stackFram;
    struct OsFpuStack *stackFpu;
    (void)taskID;

    stack = (uintptr_t *)((uintptr_t)topStack + stackSize - 1);
    stack = (uintptr_t *)((uintptr_t)stack & 0xfffffffffffffff0);

    stack[0] = 0;
    stackFram = (struct TagOsStack *)((uintptr_t)stack - sizeof(struct TagOsStack));
    memset_s(stackFram, sizeof(struct TagOsStack), 0, sizeof(struct TagOsStack));
    stackFram->rip = (U64)funcTskEntry;
    stackFram->rsp = (U64)stack;
    stackFram->rflags = 0x200;
    stackFram->cs = 0x8;
    stackFram->ss = 0x10;

    stackFpu = (struct OsFpuStack *)((uintptr_t)stackFram - OS_FPU_SIZE);
    memset_s(stackFpu, OS_FPU_SIZE, 0, OS_FPU_SIZE);
    stackFpu->fcw = 0x37f;
    stackFpu->mxcsr = 0x1f80;
    stackFpu->mxcsrMask = 0xffff;

    return (void *)stackFpu;
}

/*
 * 任务上下文获取
 */
OS_SEC_L2_TEXT void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context)
{
    *context = *((struct TskContext *)saveAddr);
    return;
}

/* 用于触发异常 */
OS_SEC_L2_TEXT void OsAsmIll(void)
{
    while (1);
}

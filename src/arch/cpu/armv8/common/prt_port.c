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
 * Create: 2022-11-22
 * Description: Hardware Initialization
 */
#include "prt_cpu_external.h"
#include "prt_sys_external.h"

#define ARMV8_X1_INIT_VALUE     0x01010101UL
#define ARMV8_X2_INIT_VALUE     0x02020202UL
#define ARMV8_X3_INIT_VALUE     0x03030303UL
#define ARMV8_X4_INIT_VALUE     0x04040404UL
#define ARMV8_X5_INIT_VALUE     0x05050505UL
#define ARMV8_X6_INIT_VALUE     0x06060606UL
#define ARMV8_X7_INIT_VALUE     0x07070707UL
#define ARMV8_X8_INIT_VALUE     0x08080808UL
#define ARMV8_X9_INIT_VALUE     0x09090909UL
#define ARMV8_X10_INIT_VALUE    0x10101010UL
#define ARMV8_X11_INIT_VALUE    0x11111111UL
#define ARMV8_X12_INIT_VALUE    0x12121212UL
#define ARMV8_X13_INIT_VALUE    0x13131313UL
#define ARMV8_X14_INIT_VALUE    0x14141414UL
#define ARMV8_X15_INIT_VALUE    0x15151515UL
#define ARMV8_X16_INIT_VALUE    0x16161616UL
#define ARMV8_X17_INIT_VALUE    0x17171717UL
#define ARMV8_X18_INIT_VALUE    0x18181818UL
#define ARMV8_X19_INIT_VALUE    0x19191919UL
#define ARMV8_X20_INIT_VALUE    0x20202020UL
#define ARMV8_X21_INIT_VALUE    0x21212121UL
#define ARMV8_X22_INIT_VALUE    0x22222222UL
#define ARMV8_X23_INIT_VALUE    0x23232323UL
#define ARMV8_X24_INIT_VALUE    0x24242424UL
#define ARMV8_X25_INIT_VALUE    0x25252525UL
#define ARMV8_X26_INIT_VALUE    0x26262626UL
#define ARMV8_X27_INIT_VALUE    0x27272727UL
#define ARMV8_X28_INIT_VALUE    0x28282828UL
#define ARMV8_X29_INIT_VALUE    0x29292929UL

#define ARMV8_SPSR_INIT_VALUE   0x305U   // EL1_SP1 | D | A | I | F
extern U32 PRT_Printf(const char *format, ...);

/* Tick中断对应的硬件定时器ID */
OS_SEC_DATA U32 g_tickTimerID = U32_INVALID;

// 系统栈配置
OS_SEC_DATA uintptr_t g_sysStackHigh[OS_VAR_ARRAY_NUM] = {(uintptr_t)&__os_sys_sp_end};
OS_SEC_DATA uintptr_t g_sysStackLow[OS_VAR_ARRAY_NUM] = {(uintptr_t)&__os_sys_sp_start};

/*
 * 描述: 将系统栈初始化为魔术字
 */
INIT_SEC_L4_TEXT void InitSystemStack(void)
{
    U32 loop;
    U32 stackSize = (U32)((uintptr_t)(&__os_sys_sp_end) - (uintptr_t)(&__os_sys_sp_start));

    /* 初始化系统栈，并写入栈魔术字 */
    for (loop = 1; loop < (stackSize / sizeof(U32)); loop++) {
        *((U32 *)((uintptr_t)&__os_sys_sp_end) - loop) = OS_SYS_STACK_TOP_MAGIC;
    }
    *((U32 *)((uintptr_t)&__os_sys_sp_start)) = OS_SYS_STACK_TOP_MAGIC;
}
/*
 * 描述: 分配各核的系统栈空间
 */
#if defined(OS_OPTION_SMP)
INIT_SEC_L4_TEXT void InitSystemSp(void)
{
    U32 loop;
    uintptr_t stackBottom = (uintptr_t)(&__os_sys_sp_end);
    U32 stackSize = (U32)((uintptr_t)(&__os_sys_sp_end) - (uintptr_t)(&__os_sys_sp_start));

    U32 stackStep = TRUNCATE((stackSize / OS_MAX_CORE_NUM), OS_TSK_STACK_SIZE_ALIGN);

    for (loop = 0; loop < OS_MAX_CORE_NUM; loop++) {
        g_sysStackHigh[loop] = stackBottom - stackStep * loop;
        g_sysStackLow[loop] = g_sysStackHigh[loop] - stackStep;
    }

    InitSystemStack();
    return;
}
#else
INIT_SEC_L4_TEXT void InitSystemSp(void)
{
    InitSystemStack();
    return;
}
#endif

/*
 * 描述: 获取系统栈的起始地址（低地址)
 */
INIT_SEC_L4_TEXT uintptr_t OsGetSysStackStart(U32 core)
{
#if defined(OS_OPTION_SMP)
    return g_sysStackLow[core];
#else
    (void)core;
    return g_sysStackLow[0];
#endif
}

/*
 * 描述: 获取系统栈的结束地址（高地址)
 */
INIT_SEC_L4_TEXT uintptr_t OsGetSysStackEnd(U32 core)
{
#if defined(OS_OPTION_SMP)
    return g_sysStackHigh[core];
#else
    (void)core;
    return g_sysStackHigh[0];
#endif
}
/*
 * 描述: 获取系统栈的栈底（高地址)
 */
OS_SEC_L0_TEXT uintptr_t OsGetSysStackSP(U32 core)
{
    return OsGetSysStackEnd(core);
}
/*
 * 描述: 初始化任务栈的上下文
 */
INIT_SEC_L4_TEXT void *OsTskContextInit(U32 taskID, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    (void)taskID;
    struct TskContext *stack = (struct TskContext *)((uintptr_t)topStack + stackSize);

    stack -= 1;

    stack->x00 = 0;
    stack->x01 = ARMV8_X1_INIT_VALUE;
    stack->x02 = ARMV8_X2_INIT_VALUE;
    stack->x03 = ARMV8_X3_INIT_VALUE;
    stack->x04 = ARMV8_X4_INIT_VALUE;
    stack->x05 = ARMV8_X5_INIT_VALUE;
    stack->x06 = ARMV8_X6_INIT_VALUE;
    stack->x07 = ARMV8_X7_INIT_VALUE;
    stack->x08 = ARMV8_X8_INIT_VALUE;
    stack->x09 = ARMV8_X9_INIT_VALUE;
    stack->x10 = ARMV8_X10_INIT_VALUE;
    stack->x11 = ARMV8_X11_INIT_VALUE;
    stack->x12 = ARMV8_X12_INIT_VALUE;
    stack->x13 = ARMV8_X13_INIT_VALUE;
    stack->x14 = ARMV8_X14_INIT_VALUE;
    stack->x15 = ARMV8_X15_INIT_VALUE;
    stack->x16 = ARMV8_X16_INIT_VALUE;
    stack->x17 = ARMV8_X17_INIT_VALUE;
    stack->x18 = ARMV8_X18_INIT_VALUE;
    stack->x19 = ARMV8_X19_INIT_VALUE;
    stack->x20 = ARMV8_X20_INIT_VALUE;
    stack->x21 = ARMV8_X21_INIT_VALUE;
    stack->x22 = ARMV8_X22_INIT_VALUE;
    stack->x23 = ARMV8_X23_INIT_VALUE;
    stack->x24 = ARMV8_X24_INIT_VALUE;
    stack->x25 = ARMV8_X25_INIT_VALUE;
    stack->x26 = ARMV8_X26_INIT_VALUE;
    stack->x27 = ARMV8_X27_INIT_VALUE;
    stack->x28 = ARMV8_X28_INIT_VALUE;
    stack->x29 = ARMV8_X29_INIT_VALUE;
    stack->x30 = funcTskEntry;   // x30： lr(link register)
    stack->xzr = 0;

    stack->elr = funcTskEntry;
    stack->esr = 0;
    stack->far = 0;
    stack->spsr = ARMV8_SPSR_INIT_VALUE;    // EL1_SP1 | D | A | I | F
    return stack;
}

/*
 * 描述: 从指定地址获取任务上下文
 */
OS_SEC_L4_TEXT void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context)
{
    *context = *((struct TskContext *)saveAddr);

    return;
}

/*
 * 描述: 手动触发异常（EL1）
 */
OS_SEC_L4_TEXT void OsAsmIll(void)
{
    OS_EMBED_ASM("svc  0");
}

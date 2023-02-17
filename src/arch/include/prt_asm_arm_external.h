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
 * Create: 2009-12-26
 * Description: ARM汇编宏， 如initRnd等。
 */
#ifndef PRT_ASM_ARM_EXTERNAL_H
#define PRT_ASM_ARM_EXTERNAL_H

#if defined(OS_ARCH_ARMV8)
#include "../cpu/armv8/common/os_asm_cpu_armv8_external.h"
#endif

/*
 * 描述 : stack_chk_guard支持用户传入seed写入函数宏
 *        argA argB argC 需要使用处传入三个可用寄存器
 */
.macro InitChkGuardRnd argA, argB, argC
    ldr \argA, = g_memCanaryRdm /* 用户传入seed */
    ldr \argB, = __stack_chk_guard
    ldr \argC, [\argA]
    cmp \argC, #0
    beq 1f
    str \argC, [\argB] /* RND写入 */
    mov \argC, #0
    str \argC, [\argA] /* 销毁 */
1:
.endm

#endif /* PRT_ASM_ARM_EXTERNAL_H */

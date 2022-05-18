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
 * Description: Macros used in assembly code
 */
#ifndef PRT_CPU_M4_EXTERNAL_H
#define PRT_CPU_M4_EXTERNAL_H

#include "prt_typedef.h"

/*
 * 模块间typedef声明
 */
typedef void (*HwiPubintFunc)(void);

#define OS_MX_IRQ_VECTOR_CNT 150
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
extern U32 g_hwiMaxNumConfig;
#define OS_HWI_MAX_NUM g_hwiMaxNumConfig
#else
#define OS_HWI_MAX_NUM       87
#define OS_HWI_FORMARRAY_NUM OS_HWI_MAX_NUM
#endif
#define OS_HWI_MAX (OS_HWI_MAX_NUM - 1) /* 最大中断号 */
#define OS_HWI_NUM_CHECK(hwiNum) ((hwiNum) > OS_HWI_MAX)
extern void g_stackEnd(void);
extern U32 g_stackStart;

#define MSTACK_VECTOR  g_stackEnd

OS_SEC_ALW_INLINE INLINE uintptr_t OsGetSysStackTop(void)
{
    return (uintptr_t)(&g_stackStart);
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsGetSysStackBottom(void)
{
    return (uintptr_t)g_stackEnd;
}

/*
 * 描述：检查配置的最大中断个数是否合法，合法返回TRUE
 */
OS_SEC_ALW_INLINE INLINE bool OsHwiCheckMaxNum(U32 maxNum)
{
    return ((maxNum > 0) && (maxNum <= OS_MX_IRQ_VECTOR_CNT));
}

#endif /* PRT_CPU_M4_EXTERNAL_H */

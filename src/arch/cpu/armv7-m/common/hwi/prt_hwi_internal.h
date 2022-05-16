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
 * Create: 2009-12-22
 * Description: hwi模块内部头文件。
 */
#ifndef PRT_HWI_INTERNAL_H
#define PRT_HWI_INTERNAL_H

#include "prt_hwi_external.h"
#include "prt_sys_external.h"
#include "prt_irq_external.h"

/*
 * 模块内宏定义
 */
#define OS_EXC_RESET                                1
#define OS_EXC_NMI                                  2
#define OS_EXC_HARD_FAULT                           3
#define OS_EXC_MPU_FAULT                            4
#define OS_EXC_BUS_FAULT                            5
#define OS_EXC_USAGE_FAULT                          6
#define OS_EXC_SVC_CALL                             11
#define OS_EXC_PEND_SV                              14
#define OS_EXC_SYS_TICK                             15

#define OS_SYS_HWI_NUM                              16
#define OS_HWI_NUM_SHIFT_BIT                        5
#define OS_NVIC_AIRCR_REG_ACCESS_PSW                0x05FA0000U
#define OS_NVIC_AIRCR_PRIGOUP_BIT_OFFSET            8  /* [10:8]位标识优先级分组 */

#define OS_HWI_ENABLE 1 /* HW使能标识 */
#define OS_HWI_CLRPEND_REG_NUM (((OS_MX_IRQ_VECTOR_CNT + 31) / 32) << 2) /* 清除PEND位寄存器数 */
#define OS_SET_VECTOR(num, vector) (g_hwiTable[(num)] = (vector))
#define OS_GET_HWI_REG_OFFSET(hwiNum) (((hwiNum) >> OS_HWI_NUM_SHIFT_BIT) << 2)

/*
 * 模块间函数声明
 */
extern void OsExcNmi(void);
extern void OsExcMemFault(void);
extern void OsExcBusFault(void);
extern void OsExcUsageFault(void);
extern void OsExcSvcCall(void);
extern void OsExcHardFault(void);
extern void OsSvchandler(void);
extern void OsResetVector(void);
extern void OsPendSv(void);
extern void OsInterrupt(void);

/* ** 向量表偏移量寄存器 VTOR */
#define OS_NVIC_VTOR 0xE000ED08UL

/* ** 中断优先级寄存器阵列 */
#define OS_NVIC_PRI_BASE 0xE000E400UL

/* ** 中断使能寄存器族 SETENA 0xE000E100-0xE000E11C */
#define OS_NVIC_SETENA_BASE 0xE000E100UL

/* ** 中断除能寄存器族 CLRENA 0xE000E180-0xE000E19C */
#define OS_NVIC_CLRENA_BASE 0xE000E180UL

/* ** 系统异常优先级寄存器阵列 */
#define OS_NVIC_EXCPRI_BASE 0xE000ED18UL

/* *** 中断控制及状态寄存器 ICSR */
#define OS_NVIC_INT_CTRL 0xE000ED04UL

/* ** 中断悬起清除寄存器(CLRPEND) 0xE000E280 - 0xE000_E29C */
#define OS_NVIC_CLRPEND_BASE 0xE000E280UL

/* ** 中断设置悬起寄存器(SETPEND) 0xE000_E200 –0xE000_E21C */
#define OS_NVIC_SETPEND_BASE 0xE000E200UL

/* 应用程序中断及复位控制寄存器 AIRCR */
#define OS_NVIC_AIRCR 0xE000ED0CUL

/* 使能中断位,寄存器置1有效，置0情况下不影响 */
#define NVIC_SET_IRQ(hwiNum)                                                                                     \
    do {                                                                                                         \
        *(volatile U32 *)((uintptr_t)OS_NVIC_SETENA_BASE + (((hwiNum) >> 5) << 2)) = 1UL << ((hwiNum) & 0x1FUL); \
    } while (0)

/* 清除中断位,寄存器置1有效，置0情况下不影响 */
#define NVIC_CLR_IRQ(hwiNum)                                                                                     \
    do {                                                                                                         \
        *(volatile U32 *)((uintptr_t)OS_NVIC_CLRENA_BASE + (((hwiNum) >> 5) << 2)) = 1UL << ((hwiNum) & 0x1FUL); \
    } while (0)

/* 设置中断优先级 */
#define NVIC_SET_IRQ_PRI(hwiNum, pri)                                         \
    do {                                                                      \
        *(volatile U8 *)((uintptr_t)OS_NVIC_PRI_BASE + (hwiNum)) = (U8)(pri); \
    } while (0)

/* 设置异常优先级 */
#define NVIC_SET_EXC_PRI(excNum, pri)                                                  \
    do {                                                                               \
        *(volatile U8 *)((uintptr_t)OS_NVIC_EXCPRI_BASE + ((excNum) - 4)) = (U8)(pri); \
    } while (0)

/* 清除悬起中断位 */
#define NVIC_CLR_IRQ_PEND(hwiNum)                                                                                 \
    do {                                                                                                          \
        *(volatile U32 *)((uintptr_t)OS_NVIC_CLRPEND_BASE + (((hwiNum) >> 5) << 2)) = 1UL << ((hwiNum) & 0x1FUL); \
    } while (0)

/* 设置悬起中断位 */
#define NVIC_SET_IRQ_PEND(hwiNum)                                                                                 \
    do {                                                                                                          \
        *(volatile U32 *)((uintptr_t)OS_NVIC_SETPEND_BASE + (((hwiNum) >> 5) << 2)) = 1UL << ((hwiNum) & 0x1FUL); \
    } while (0)

#endif /* PRT_HWI_INTERNAL_H */

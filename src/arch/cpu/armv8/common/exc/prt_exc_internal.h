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
 * Description: 异常模块内部头文件。
 */
#ifndef PRT_EXC_INTERNAL_H
#define PRT_EXC_INTERNAL_H

#include "prt_attr_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_exc_external.h"
#include "prt_irq_external.h"
#include "prt_task_external.h"
#include "prt_hook_external.h"
#include "prt_mem_external.h"
#include "prt_sem_external.h"
#include "prt_task_external.h"
#include "prt_queue_external.h"
#include "prt_swtmr_external.h"
#include "prt_timer_external.h"
#include "prt_raw_spinlock_external.h"
#if defined(OS_OPTION_SMP)
#include "prt_sched_external.h"
#endif
/*
 * 模块内宏定义
 */
#define OS_EXC_DEFAULT_EXC_TYPE 0

#define OS_EXC_STACK_ALIGN     0x10U

#define OS_EXC_ESR_UNDEF_INSTR  0x20U
#define OS_EXC_ESR_PC_NOT_ALIGN 0x22U
#define OS_EXC_ESR_DATA_ABORT   0x24U
#define OS_EXC_ESR_SP_INSTR     0x26U

#define INVALIDPID         0xFFFFFFFFUL
#define INVALIDSTACKBOTTOM 0xFFFFFFFFUL

/*
 * 模块内结构体定义
 */
/* information for esr */
union TagExcEsr {
    struct {
        uintptr_t iss  : 25; // bit[0:24] The Instruction specific syndrome field.
        uintptr_t il   : 1;  // bit[25] 指令长度位，用于同步异常，0:16-bit; 0:32-bit
        uintptr_t ec   : 6;  // bit[26:31] Exception class 标记错误原因
        uintptr_t res0 : 32; // bit[32:63]
    } bits;
    uintptr_t esr;
};

/*
 * 模块内全局变量声明
 */
extern U32 OsVectorTable(void);

/*
 * 模块内函数声明
 */
extern void OsExcSaveInfo(struct ExcInfo *excInfo, struct ExcRegInfo *regs);

#endif /* PRT_EXC_INTERNAL_H */

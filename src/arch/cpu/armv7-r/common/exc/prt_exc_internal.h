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

#ifndef __ASSEMBLER__
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

/**
 * system sections start and end address
 */
extern char __int_stack_start;
extern char __int_stack_end;
extern char __rodata_start;
extern char __rodata_end;
extern char __bss_start;
extern char __bss_end;
extern char __text_start;
extern char __text_end;
extern char __ram_data_start;
extern char __ram_data_end;
extern char __exc_heap_start;
extern char __exc_heap_end;
extern char __heap_start;
extern char __init_array_start__;
extern char __init_array_end__;

void OsExcType(U32 excType, struct ExcRegInfo *excRegs);
void OsExcSysInfo(U32 excType, const struct ExcRegInfo *excRegs);
void OsExcRegsInfo(struct ExcRegInfo *excBufAddr);
void OsCallStackInfo(void);
#else
#include <prt_asm_arm_external.h>
#endif

/* Define exception type ID */
#define OS_EXCEPT_RESET          0x00
#define OS_EXCEPT_UNDEF_INSTR    0x01
#define OS_EXCEPT_SWI            0x02
#define OS_EXCEPT_PREFETCH_ABORT 0x03
#define OS_EXCEPT_DATA_ABORT     0x04
#define OS_EXCEPT_FIQ            0x05
#define OS_EXCEPT_ADDR_ABORT     0x06
#define OS_EXCEPT_IRQ            0x07

#endif /* PRT_EXC_INTERNAL_H */

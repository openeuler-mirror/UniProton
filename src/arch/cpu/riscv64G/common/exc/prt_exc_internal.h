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
 * Create: 2024-01-13
 * Description: Hardware interrupt implementation
 */
#ifndef PRT_EXC_INTERNAL_H
#define PRT_EXC_INTERNAL_H

#include "prt_exc_external.h"


#define OS_EXC_DEFAULT_EXC_TYPE 0

#define OS_EXC_STACK_ALIGN     0x10U

#define OS_EXC_ESR_UNDEF_INSTR  0x20U
#define OS_EXC_ESR_PC_NOT_ALIGN 0x22U
#define OS_EXC_ESR_DATA_ABORT   0x24U
#define OS_EXC_ESR_SP_INSTR     0x26U

#define INVALIDPID         0xFFFFFFFFUL

#define OS_EXC_MAX_NEST_DEPTH   4

extern void OsExcSaveInfo(struct ExcInfo *excInfo, struct ExcCalleeInfo *calleeInfo, struct ExcCauseRegInfo* regInfo);
extern void OsExcHandleEntryRISCV();

#endif
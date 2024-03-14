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
 * Description: Hardware interrupt implementation
 */
#ifndef PRT_HWI_EXTERNAL_H
#define PRT_HWI_EXTERNAL_H

#include "prt_hwi.h"
#include "prt_buildef.h"
#define OS_HWI_NUM_MASK 0x1FU

/*
 * 模块间全局变量声明
 */
extern U8 g_hwiNum[];

/*
 * 模块间函数声明
 */
extern void OsHwiGICInit(void);
extern U32 OsHwiPriorityGet(HwiHandle hwiNum);
extern void OsHwiPrioritySet(HwiHandle hwiNum, HwiPrior hwiPrio);
#if !defined(OS_OPTION_SMP)
extern void OsHwiMcTrigger(U32 coreMask, U32 hwiNum);
#endif
extern void OsHwiDisableAll(void);

#endif /* PRT_HWI_EXTERNAL_H */

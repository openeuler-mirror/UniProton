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
 * Description: hw模块内部头文件。
 */
#ifndef PRT_EXC_INTERNAL_H
#define PRT_EXC_INTERNAL_H

#include "prt_exc_external.h"

/*
 * 模块内宏定义
 */
#define OS_EXC_MAX_NEST_DEPTH                5
#define OS_EXC_FLAG_FAULTADDR_VALID          0x01U /* 异常类型:faultAddr域是否有效 标志位 */
#define OS_EXC_IMPRECISE_ACCESS_ADDR         0xABABABABUL

#define INVALIDPID                      0xFFFFFFFFUL
#define INVALIDSTACKBOTTOM              0xFFFFFFFFUL

extern void OsExcSaveInfo(struct ExcRegInfo *regs);
extern void OsExcHandleEntryM4(U32 excType, U32 faultAddr, struct ExcContext *excBufAddr);

#endif /* PRT_EXC_INTERNAL_H */

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
 * Description: exc模块内部头文件
 */
#ifndef PRT_KEXC_EXTERNAL_H
#define PRT_KEXC_EXTERNAL_H

#include "prt_exc.h"
#include "prt_cpu_external.h"
/*
 * 模块内宏定义
 */
#define EXC_RECORD_SIZE (sizeof(struct ExcInfo))
#define OS_EXC_INFO_ADDR (&EXC_INFO_INTERNAL.excInfo)

/*
 * 模块内数据结构定义
 */
struct TagExcInfoInternal {
    /* 异常信息 */
    struct ExcInfo excInfo;
};

/*
 * 模块内全局变量声明
 */
extern struct ExcModInfo g_excModInfo;

#if !defined(OS_OPTION_SMP)
extern U32 g_curNestCount;
#define CUR_NEST_COUNT g_curNestCount

extern struct TagExcInfoInternal g_excInfoInternal;
#define EXC_INFO_INTERNAL g_excInfoInternal
#else
extern U32 g_curNestCount[OS_MAX_CORE_NUM];
#define CUR_NEST_COUNT (g_curNestCount[THIS_CORE()])

extern struct TagExcInfoInternal g_excInfoInternal[OS_MAX_CORE_NUM];
#define EXC_INFO_INTERNAL (g_excInfoInternal[THIS_CORE()])
#endif

#endif /* PRT_KEXC_EXTERNAL_H */

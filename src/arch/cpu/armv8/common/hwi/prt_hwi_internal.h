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
 * Description: hwi模块内部头文件。
 */
#ifndef PRT_HWI_INTERNAL_H
#define PRT_HWI_INTERNAL_H

#include "prt_cpu_external.h"

/*
 * 模块内 内联函数
 */
#if defined(OS_OPTION_HWI_NESTED)
/*
 * 描述: 支持中断嵌套场景--进入ISR前开中断
 */
OS_SEC_ALW_INLINE INLINE void OsHwiNestedIntEnable(void)
{
    OsIntEnable();
}

/*
 * 描述: 支持中断嵌套场景--退出ISR后关中断
 */
OS_SEC_ALW_INLINE INLINE void OsHwiNestedIntDisable(void)
{
    OsIntDisable();
}
#else
OS_SEC_ALW_INLINE INLINE void OsHwiNestedIntEnable(void)
{
}

OS_SEC_ALW_INLINE INLINE void OsHwiNestedIntDisable(void)
{
}
#endif /* OS_OPTION_HWI_NESTED */

#endif /* PRT_HWI_INTERNAL_H */

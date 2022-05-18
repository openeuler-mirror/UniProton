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
 * Description: CPU占用率模块的C文件
 */
#include "prt_cpup_internal.h"

/*
 * 描述：获取当前cpu占用率。
 */
OS_SEC_L2_TEXT U32 PRT_CpupNow(void)
{
    U32 ret;

    /* 如果没有初始化，则进行CPUP初始化 */
    ret = OsCpupLazyInit();
    if (ret != OS_OK) {
        OS_REPORT_ERROR(ret);
        return (U32)OS_INVALID;
    }

    return g_cpupNow();
}


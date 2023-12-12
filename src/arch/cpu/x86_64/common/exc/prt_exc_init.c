/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-18
 * Description: 异常处理（初始化相关）。
 */
#include "prt_typedef.h"
#include "prt_attr_external.h"
#include "prt_exc_external.h"

OS_SEC_BSS ExcTaskInfoFunc g_excTaskInfoGet;
/*
 * 描述: EXC模块的初始化
 */
OS_SEC_L4_TEXT U32 OsExcConfigInit(void)
{
    return OS_OK;
}

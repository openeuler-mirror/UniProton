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
 * Create: 2024-06-10
 * Description: Trace模块注册和初始化。
 */
#include "prt_trace_internal.h"
#include "prt_hook_external.h"

#ifdef OS_OPTION_TRACE

U32 OsTraceConfigReg(void)
{
    OsMhookReserve((U32)OS_HOOK_TSK_SWITCH, 1);
    OsMhookReserve((U32)OS_HOOK_TSK_CREATE, 1);
    OsMhookReserve((U32)OS_HOOK_TSK_DELETE, 1);
    OsMhookReserve((U32)OS_HOOK_HWI_ENTRY, 1);
    OsMhookReserve((U32)OS_HOOK_HWI_EXIT, 1);
    return OS_OK;
}

U32 OsTraceConfigInit(void)
{
    return OsTraceInit();
}

#endif /* OS_OPTION_TRACE */
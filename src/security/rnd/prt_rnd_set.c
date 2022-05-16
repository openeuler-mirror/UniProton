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
 * Create: 2009-06-20
 * Description: 随机化功能的C文件。
 */
#include "prt_sys.h"
#include "prt_attr_external.h"

OS_SEC_L4_DATA U32 g_memCanaryRdm = 0;

OS_SEC_L4_TEXT U32 PRT_SysSetRndNum(enum SysRndNumType type, U32 rndNum)
{
    switch (type) {
        case OS_SYS_RND_STACK_PROTECT:
            g_memCanaryRdm = rndNum;
            break;
        default:
            return OS_ERRNO_SYS_RND_PARA_INVALID;
    }
    return OS_OK;
}

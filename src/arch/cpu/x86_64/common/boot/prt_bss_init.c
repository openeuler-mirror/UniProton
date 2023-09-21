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
 * Description: BSS初始化。
 */
#include "prt_cpu_external.h"
#include "securec.h"

/*
 * 描述: BSS段初始化
 */
extern unsigned long long __bss_end;
extern unsigned long long __bss_start;
INIT_SEC_L4_TEXT void InitBssOs(void)
{
    uintptr_t len = (uintptr_t)(&__bss_end) - (uintptr_t)(&__bss_start);
    if (memset_s((void *)&__bss_start, len, 0, len) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    return;
}
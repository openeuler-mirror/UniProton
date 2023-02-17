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
 * Description: BSS初始化。
 */
#include "prt_cpu_external.h"
/*
 * 描述: BSS段初始化
 */
INIT_SEC_L4_TEXT void OsBssInit(void)
{
    uintptr_t len = (uintptr_t)(&__bss_end__) - (uintptr_t)(&__bss_start__);
    if (memset_s((void *)&__bss_start__, len, 0, len) != EOK) {
        OS_GOTO_SYS_ERROR();
    }

    return;
}
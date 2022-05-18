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
 * Description: tick后处理调度文件。
 */
#include "prt_sys_external.h"

OS_SEC_ALW_INLINE INLINE void OsTickBh(void)
{
    uintptr_t intSave;

    while (TICK_NO_RESPOND_CNT > 0) {
        TICK_NO_RESPOND_CNT--;
        UNI_FLAG |= OS_FLG_TICK_ACTIVE;

        /* 整个调度过程中就是关中断的 */
        intSave = PRT_HwiUnLock();

        /* 如果tick未初始化，TICK_NO_RESPOND_CNT=0，故不会进入此处，调用tick后处理 */
        g_tickDispatcher();

        PRT_HwiRestore(intSave);
        UNI_FLAG &= ~OS_FLG_TICK_ACTIVE;
    }
}

OS_SEC_TEXT void OsViDispatch(void)
{
    /* tick 后处理的调度 */
    if ((UNI_FLAG & OS_FLG_TICK_ACTIVE) != 0) {
        return;
    }

    OsTickBh();
}

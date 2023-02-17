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
 * Create: 2022-11-15
 * Description: 获取软件定时器信息功能
 */
#include "prt_swtmr_internal.h"

OS_SEC_L4_TEXT U32 PRT_SwTmrInfoGet(TimerHandle tmrHandle, struct SwTmrInfo *info)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;
    U32 expireTime = 0;

    if (g_timerApi[TIMER_TYPE_SWTMR].createTimer == NULL) {
        return OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
    }

    if (tmrHandle < OS_SWTMR_MIN_NUM) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    if (tmrHandle >= OS_TIMER_MAX_NUM) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    if (info == NULL) {
        return OS_ERRNO_SWTMR_RET_PTR_NULL;
    }

    intSave = OsIntLock();

    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    if (swtmr->state == (U8)OS_TIMER_FREE) {
        OsIntRestore(intSave);
        return OS_ERRNO_SWTMR_NOT_CREATED;
    }

    /* 写入返回参数 */
    info->handler = swtmr->handler;
    info->mode = (enum TmrMode)swtmr->mode;
    info->state = (U8)((U32)swtmr->state & OS_SWTMR_STATUS_MASK);
    // Interval转化为毫秒一定为整数
    info->interval = (U32)DIV64(((U64)swtmr->interval * OS_SYS_MS_PER_SECOND), g_tickModInfo.tickPerSecond);

    /* 调用内部接口osSwTmrQuery获取剩余时间 */
    OsErrRecord(OsSwTmrQuery(tmrHandle, &expireTime));

    info->remainMs = expireTime;
    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-03-18
 */

#include "prt_perf_pmu.h"

OS_SEC_BSS static Pmu *g_pmuMgr[PERF_EVENT_TYPE_MAX];

U32 OsPerfPmuRegister(Pmu *pmu)
{
    U32 type;

    if ((pmu == NULL) || (pmu->type >= PERF_EVENT_TYPE_MAX)) {
        return OS_ERROR;
    }

    type = pmu->type;
    if (g_pmuMgr[type] == NULL) {
        g_pmuMgr[type] = pmu;
        return OS_OK;
    }

    return OS_ERROR;
}

Pmu *OsPerfPmuGet(U32 type)
{
    if (type >= PERF_EVENT_TYPE_MAX) {
        return NULL;
    }

    /* process hardware raw events with hard pmu */
    if (type == PERF_EVENT_TYPE_RAW) {
        type = PERF_EVENT_TYPE_HW;
    }

    return g_pmuMgr[type];
}

void OsPerfPmuRm(U32 type)
{
    if (type >= PERF_EVENT_TYPE_MAX) {
        return;
    }
    g_pmuMgr[type] = NULL;
    return;
}
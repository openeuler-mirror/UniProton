/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Description: Power manager implementation header, ported from LiteOS los_lowpower_impl.h.
 */
#ifndef PRT_LOWPOWER_IMPL_H
#define PRT_LOWPOWER_IMPL_H

#include "prt_lowpower.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct PowerMgrRunOps {
    void (*changeFreq)(U8 freq);
    void (*enterLightSleep)(U32 sleepTicks);
    void (*enterDeepSleep)(void);
    void (*setWakeUpTimer)(U32 timeout);
    U32 (*withdrawWakeUpTimer)(void);
    U32 (*getSleepTime)(void);
    U32 (*selectSleepMode)(U32 sleepTicks);
    U32 (*preConfig)(void);
    void (*postConfig)(void);
    void (*contextSave)(void);
    void (*contextRestore)(void);
    U32 (*getDeepSleepVoteCount)(void);
    U32 (*getSleepMode)(void);
    void (*setSleepMode)(U32 mode);
};

struct PowerMgrConfig {
    U32 minLightSleepTicks;
    U32 minDeepSleepTicks;
    U32 maxDeepSleepTicks;
};

struct PowerMgrDeepSleepOps {
    bool (*couldDeepSleep)(void);
    void (*systemWakeup)(void);
    bool (*suspendPreConfig)(void);
    void (*suspendDevice)(void);
    void (*rollback)(void);
    void (*resumeDevice)(void);
    void (*resumePostConfig)(void);
    void (*resumeCallBack)(void);
    void (*otherCoreResume)(void);
    void (*resumeFromReset)(void);
};

struct PowerMgrParameter {
    struct PowerMgrRunOps runOps;
    struct PowerMgrDeepSleepOps deepSleepOps;
    struct PowerMgrConfig config;
};

struct PowerMgrStats {
    U32 processCount;
    U32 lightSleepCount;
    U32 deepSleepCount;
    U32 deepSaveCount;
    U32 deepRollbackCount;
    U32 deepResumeCount;
};

void PRT_PowerMgrInit(const struct PowerMgrParameter *para);
void PRT_PowerMgrStatsGet(struct PowerMgrStats *stats);
void OsPmSetReady(void);

/* Compatibility aliases for the existing sd3403 BSP code during migration. */
void OsPmInit(const struct PowerMgrParameter *para);
void OsPmProcess(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_LOWPOWER_IMPL_H */

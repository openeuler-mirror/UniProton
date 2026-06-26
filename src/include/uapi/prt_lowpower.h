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
 * Description: Low-power framework external header, ported from LiteOS los_lowpower.h.
 */
#ifndef PRT_LOWPOWER_H
#define PRT_LOWPOWER_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

enum PRT_IntermitMode {
    PRT_INTERMIT_NONE = 0,
    PRT_INTERMIT_LIGHT_SLEEP,
    PRT_INTERMIT_DEEP_SLEEP,
    PRT_INTERMIT_STANDBY,
    PRT_INTERMIT_SHUTDOWN,
    PRT_INTERMIT_MAX,
};

enum PRT_FreqMode {
    PRT_SYS_FREQ_SUPER = 0,
    PRT_SYS_FREQ_HIGH,
    PRT_SYS_FREQ_NORMAL,
    PRT_SYS_FREQ_LOW,
    PRT_SYS_FREQ_MAX,
};

typedef U32 (*LowpowerExternalVoterHandle)(void);
typedef void (*LowPowerHookFn)(void);
typedef void (*IntWakeupHookFn)(U32 hwiNum);

struct PowerMgrOps {
    void (*process)(void);
    void (*wakeupFromReset)(void);
    void (*resumeFromInterrupt)(U32 hwiNum);
    void (*changeFreq)(enum PRT_FreqMode freq);
    void (*deepSleepVoteBegin)(void);
    void (*deepSleepVoteEnd)(void);
    void (*deepSleepVoteDelay)(U32 tick);
    void (*registerExternalVoter)(LowpowerExternalVoterHandle handler);
    U32 (*getDeepSleepVoteCount)(void);
    U32 (*getSleepMode)(void);
    void (*setSleepMode)(U32 mode);
};

void PRT_LowpowerInit(const struct PowerMgrOps *pmOps);
void PRT_LowpowerHookReg(LowPowerHookFn hook);
void PRT_IntWakeupHookReg(IntWakeupHookFn hook);
void OsLowpowerIntWakeupHookCall(U32 hwiNum);
void OsPowerMgrProcess(void);
void OsPowerMgrWakeUpFromInterrupt(U32 hwiNum);
void OsPowerMgrWakeupFromReset(void);

void PRT_PowerMgrChangeFreq(enum PRT_FreqMode freq);
void PRT_PowerMgrDeepSleepVoteBegin(void);
void PRT_PowerMgrDeepSleepVoteEnd(void);
void PRT_PowerMgrSleepDelay(U32 tick);
void PRT_PowerMgrRegisterExtVoter(LowpowerExternalVoterHandle handler);
U32 PRT_PowerMgrGetSleepMode(void);
U32 PRT_PowerMgrGetDeepSleepVoteCount(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_LOWPOWER_H */

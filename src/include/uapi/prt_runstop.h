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
 * Description: Run-stop (suspend-to-RAM/flash) header. The flow follows LiteOS
 *              los_runstop.c, with the storage backend abstracted behind
 *              RunstopStorageOps so that the kernel does not depend on a
 *              particular flash driver.
 */
#ifndef PRT_RUNSTOP_H
#define PRT_RUNSTOP_H

#include "prt_typedef.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(OS_OPTION_RUNSTOP)

#define RUNSTOP_OK     0
#define RUNSTOP_FAIL   1

#define RUNSTOP_MAGIC        0x57574F57U
#define RUNSTOP_COLD_BOOT    0
#define RUNSTOP_RESUME       1
#define RUNSTOP_RESUME_MAGIC 0x52534D45U

#define RUNSTOP_WOW_PAYLOAD_OFFSET  0x1000U

typedef struct {
    U32 magic;
    U32 reserved;
    U64 dataStart;
    U64 dataSize;
    U64 bssStart;
    U64 bssSize;
    U64 saveSRContext[34];
} WowHeader;

typedef struct {
    U32 magic;
    U32 imageSize;
    U64 dataSize;
    U64 bssSize;
    U32 resumeFlag;
    U32 reserved;
} WowShmHeader;

typedef struct {
    U32 imageSize;
    U64 dataStart;
    U64 dataSize;
    U64 bssStart;
    U64 bssSize;
    U64 srContextAddr;
    U64 srContextSize;
    U64 tickCount;
} RunstopImageInfo;

struct RunstopStorageOps {
    U32 (*getImageInfo)(RunstopImageInfo *info);
    U32 (*validateImage)(void);
    U32 (*saveImage)(const RunstopImageInfo *info);
    void (*restoreImage)(void);
};

extern U32 g_wowMagic;
extern U64 g_wowDataSize;
extern U64 g_wowBssSize;

void OsRunstopStateInit(void);
void OsRunstopStorageOpsRegister(const struct RunstopStorageOps *ops);
U32 OsRunstopBackendInit(void);

U32 OsRunstopInit(void);
U32 OsRunstopResumeCheck(void);
U32 OsRunstopMakeImage(void);
void OsRunstopSuspend(void);
U32 OsRunstopStorePending(void);
void OsRunstopStoreSystemInfoBeforeSuspend(void);

/* Hooks consumed by the power-manager framework (mirror LiteOs OsWowSysDoneFlagGet /
 * OsSystemWakeup), named with the PRT runstop prefix. */
U32 OsRunstopWowSysDoneFlagGet(void);
void OsRunstopSystemWakeup(void);

/*
 * Pluggable low-power hooks used while a runstop image is being taken. By
 * default the deep-sleep entry is a WFI loop woken by a timer (the board runs
 * inside a mica instance, so a true power-gating deep sleep cannot be driven
 * from the RTOS without Linux). A customer that needs real deep sleep registers
 * its own enterDeepSleep (and the reset-vector resume path that calls
 * OsRunstopSystemWakeup / OsLowpowerRestoreRegister). This mirrors LiteOs,
 * whose default OsEnterDeepSleepDefault / OsLightSleepDefault are also wfi().
 */
struct RunstopLowpowerOps {
    void (*enterDeepSleep)(void);      /* NULL => WFI loop on g_runstopWakeFlag */
    void (*setWakeUpTimer)(U32 ticks); /* configure the wake source */
    void (*withdrawWakeUpTimer)(void); /* tear down the wake source */
};

extern volatile U32 g_runstopWakeFlag; /* set by the wake source */

void OsRunstopLowpowerOpsReg(const struct RunstopLowpowerOps *ops);
void OsRunstopSetSleepTicks(U32 ticks);

/* Shared with the power-manager module (defined in prt_lowpower_impl.c). */
extern volatile U32 g_lowpowerOtherCoreResume;

#endif /* OS_OPTION_RUNSTOP */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_RUNSTOP_H */

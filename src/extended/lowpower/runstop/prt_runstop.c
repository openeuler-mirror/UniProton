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
 * Description: Run-stop (suspend-to-storage) module. The control flow follows
 *              LiteOs los_runstop.c (OsSystemSuspend / OsWriteToFlashTask /
 *              OsWaitImagingDone / OsStoreSystemInfoBeforeSuspend /
 *              OsSystemWakeup). Kernel interface dependencies are replaced with
 *              UniProton PRT_ equivalents:
 *                - LiteOs independent event control block (EVENT_CB_S) is
 *                  replaced by a counting semaphore (PRT_Sem*), because
 *                  UniProton events are per-task and not standalone CBs.
 *                - The flash read/write function pointers are abstracted behind
 *                  RunstopStorageOps (saveImage / restoreImage / validateImage /
 *                  getImageInfo), so the kernel does not depend on a particular
 *                  flash driver or memory-layout symbols.
 */
#include "prt_runstop.h"
#include "prt_lowpower_context.h"
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_sem.h"
#include "prt_tick.h"
#include "prt_cpu_external.h"
#include "prt_errno.h"
#include "prt_mem.h"

#if defined(OS_OPTION_RUNSTOP)

extern U32 PRT_Printf(const char *format, ...);

/* Real RAM region boundaries (from the sd3403 linker script). __bss_start__/
 * __bss_end__ are already declared as uintptr_t by os_cpu_armv8_external.h. */
extern U8 __data_start[];
extern U8 __data_end[];

#define WAKEUP_FROM_SUSPEND 0x01U
#define FLASH_IMG_SUCCESS   0x02U

#define OS_NO_STORE_SYSTEM  0U
#define OS_STORE_SYSTEM    1U

OS_SEC_BSS U32 g_wowMagic = 0;
OS_SEC_BSS U64 g_wowDataSize = 0;
OS_SEC_BSS U64 g_wowBssSize = 0;

/* Per-core imaging request flag, mirrors LiteOs g_sysDoneFlag. */
OS_SEC_BSS static U32 g_sysDoneFlag[OS_MAX_CORE_NUM];

OS_SEC_BSS static const struct RunstopStorageOps *g_storageOps = NULL;
OS_SEC_BSS static const struct RunstopLowpowerOps *g_lowpowerOps = NULL;
OS_SEC_BSS static SemHandle g_suspendResumeSem = 0;
OS_SEC_BSS static SemHandle g_writeFlashSem = 0;
OS_SEC_BSS static U32 g_runstopResult = 0;
OS_SEC_BSS static bool g_runstopInited = false;
OS_SEC_BSS static TskHandle g_writeFlashTaskId = 0;
OS_SEC_BSS static U32 g_runstopSleepTicks = 50;   /* default simulated suspend length */

/* Set by the wake source; the default deep-sleep loop exits when it goes non-zero. */
OS_SEC_BSS volatile U32 g_runstopWakeFlag = 0;

static inline void OsRunstopAsmWfi(void)
{
#if defined(OS_ARCH_ARMV8) || defined(OS_ARCH_ARMV7) || defined(OS_ARCH_AARCH32)
    OS_EMBED_ASM("wfi" : : : "memory");
#endif
}

/* Default deep-sleep entry: WFI loop woken by the hardware tick timer. Each WFI
 * returns on the next tick IRQ, so the loop suspends for g_runstopSleepTicks
 * ticks. The tick is an interrupt, so it wakes WFI even while PRT_TaskLock is
 * held. A customer needing real (power-gating) deep sleep registers its own
 * enterDeepSleep (plus the reset-vector resume path). This mirrors LiteOs whose
 * default OsEnterDeepSleepDefault is also wfi(). */
static void OsRunstopEnterDeepSleepDefault(void)
{
    U32 i;
    for (i = 0; i < g_runstopSleepTicks; i++) {
        OsRunstopAsmWfi();
    }
}

void OsRunstopLowpowerOpsReg(const struct RunstopLowpowerOps *ops)
{
    g_lowpowerOps = ops;
}

void OsRunstopSetSleepTicks(U32 ticks)
{
    g_runstopSleepTicks = (ticks == 0) ? 1 : ticks;
}

static void OsRunstopEnterDeepSleep(void)
{
    if ((g_lowpowerOps != NULL) && (g_lowpowerOps->enterDeepSleep != NULL)) {
        g_lowpowerOps->enterDeepSleep();
    } else {
        OsRunstopEnterDeepSleepDefault();
    }
}

static void OsRunstopSetWakeUpTimer(void)
{
    if ((g_lowpowerOps != NULL) && (g_lowpowerOps->setWakeUpTimer != NULL)) {
        g_lowpowerOps->setWakeUpTimer(g_runstopSleepTicks);
    }
}

static void OsRunstopWithdrawWakeUpTimer(void)
{
    if ((g_lowpowerOps != NULL) && (g_lowpowerOps->withdrawWakeUpTimer != NULL)) {
        g_lowpowerOps->withdrawWakeUpTimer();
    }
}

static void OsRunstopSemInit(void)
{
    (void)PRT_SemCreate(0, &g_suspendResumeSem);
    (void)PRT_SemCreate(0, &g_writeFlashSem);
}

bool IsImageResume(void)
{
    return (g_lowpowerResumeFromImg != LOWPOWER_COLD_RESET);
}

/* Populate the image descriptor with the real runtime RAM regions, the saved
 * register context and the tick count. The storage backend persists whatever
 * this describes, so the saved image is a genuine snapshot of system state. */
static void OsRunstopFillImageInfo(RunstopImageInfo *info)
{
    if (info == NULL) {
        return;
    }
    info->dataStart = (U64)(uintptr_t)__data_start;
    info->dataSize = (U64)(__data_end - __data_start);
    info->bssStart = (U64)(uintptr_t)&__bss_start__;
    info->bssSize = (U64)((uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__);
    info->srContextAddr = (U64)(uintptr_t)g_lowpowerSaveSRContext;
    info->srContextSize = (U64)sizeof(g_lowpowerSaveSRContext);
    info->tickCount = PRT_TickGetCount();
    info->imageSize = (U32)(info->dataSize + info->bssSize + info->srContextSize);
}

void OsRunstopStateInit(void)
{
    for (U32 i = 0; i < OS_MAX_CORE_NUM; i++) {
        g_sysDoneFlag[i] = OS_NO_STORE_SYSTEM;
    }
    g_runstopResult = 0;
    OsLowpowerContextInit();
    OsRunstopSemInit();
}

void OsRunstopStorageOpsRegister(const struct RunstopStorageOps *ops)
{
    g_storageOps = ops;
}

U32 OsRunstopBackendInit(void)
{
    /* On a real resume the storage backend validates the persisted image. */
    if ((g_storageOps != NULL) && (g_storageOps->validateImage != NULL) &&
        (g_lowpowerResumeFromImg == LOWPOWER_RUNSTOP_RESET)) {
        if (g_storageOps->validateImage() != RUNSTOP_OK) {
            return RUNSTOP_FAIL;
        }
    }
    return RUNSTOP_OK;
}

U32 OsRunstopResumeCheck(void)
{
    if (g_lowpowerResumeFromImg == LOWPOWER_RUNSTOP_RESET) {
        return RUNSTOP_RESUME;
    }
    return RUNSTOP_COLD_BOOT;
}

/*
 * Two suspend modes, mirroring LiteOs:
 *  - Imaging suspend (LiteOs OsSystemSuspend, reached when LOS_MakeImage set the
 *    resume-from-image flag via the idle hook + write-flash task):
 *      save CPU context -> save real RAM image -> set wake source -> enter deep
 *      sleep (default WFI loop woken by the wake source; customer plugs real deep
 *      sleep + reset-resume) -> wake -> withdraw wake source -> compensate tick ->
 *      signal OsWaitImagingDone -> return inline.
 *  - Light suspend (LiteOs OsLightSleepDefault, reached when no imaging is
 *    pending): a single wfi(), rest until the next interrupt. No imaging, no
 *    tickless, no OsWaitImagingDone handshake (no make-image caller is waiting,
 *    so the suspend-resume sem is NOT posted).
 *
 * OsLowpowerRestoreRegister is NOT called on either default path: with WFI RAM is
 * retained, so execution simply continues after WFI. It is reserved for a
 * customer's real (power-gating) deep sleep, whose reset-vector resume path calls
 * OsRunstopSystemWakeup. This mirrors LiteOs, where OsSRRestoreRegister lives
 * only in the reset-resume path (OsDeepSleepResume), not in the default-wfi path.
 */
void OsRunstopSuspend(void)
{
    U32 cpuid;
    bool postSem = false;
    uintptr_t intSave = OsIntLock();

    PRT_TaskLock();
    cpuid = OsGetCoreID();
    g_sysDoneFlag[cpuid] = OS_NO_STORE_SYSTEM;

    /* 1. Save CPU register context (practiced here; restored only on a real
     *    power-gating deep sleep's reset-resume path). */
    OsLowpowerSaveRegister();

    if (IsImageResume()) {
        /* 2. Persist a genuine snapshot of .data/.bss/registers/tick. */
        if ((g_storageOps != NULL) && (g_storageOps->saveImage != NULL)) {
            RunstopImageInfo info = {0};
            OsRunstopFillImageInfo(&info);
            if (g_storageOps->getImageInfo != NULL) {
                (void)g_storageOps->getImageInfo(&info);
            }
            (void)g_storageOps->saveImage(&info);
            g_wowDataSize = info.dataSize;
            g_wowBssSize = info.bssSize;
        }

        /* 3. Configure the wake source, then enter (simulated) deep sleep. */
        U64 tickBefore = PRT_TickGetCount();
        OsRunstopSetWakeUpTimer();
        OsRunstopEnterDeepSleep();
        OsRunstopWithdrawWakeUpTimer();
        U64 tickAfter = PRT_TickGetCount();

        /* 4. Compensate the tick for ticks that did NOT fire during suspend.
         *    With the default WFI simulation the tick keeps running, so elapsed
         *    ~= sleepTicks and the compensation is ~0. With a real (power-gating)
         *    deep sleep the tick is suppressed, elapsed ~= 0 and the full
         *    sleepTicks are compensated. This is the LiteOs OsSysTimeUpdate step. */
        U32 elapsed = (tickAfter > tickBefore) ? (U32)(tickAfter - tickBefore) : 0;
        if ((U32)g_runstopSleepTicks > elapsed) {
            (void)PRT_TickCountAdjust((S32)((U32)g_runstopSleepTicks - elapsed));
        }

        g_runstopResult = FLASH_IMG_SUCCESS;
        /* Inline resume (no reset): signal the make-image caller. LiteOs
         * OsWaitImagingDone handshake. */
        postSem = true;
    } else {
        /* Light suspend (LiteOs OsLightSleepDefault = wfi): rest until the next
         * interrupt, then return. No imaging handshake is pending. */
        OsRunstopAsmWfi();
        g_runstopResult = WAKEUP_FROM_SUSPEND;
    }

    PRT_TaskUnlock();
    OsIntRestore(intSave);
    if (postSem) {
        (void)PRT_SemPost(g_suspendResumeSem);
    }
}

/*
 * LiteOs OsWriteToFlashTask: dedicated task that performs the imaging when
 * triggered by OsRunstopStoreSystemInfoBeforeSuspend.
 */
static void OsWowWriteFlashTask(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    while (1) {
        (void)PRT_SemPend(g_writeFlashSem, OS_WAIT_FOREVER);
        OsRunstopSuspend();
    }
}

static U32 OsWowWriteFlashTaskCreate(void)
{
    struct TskInitParam param = {0};
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    if (param.stackAddr == 0) {
        return OS_ERROR;
    }
    param.taskEntry = (TskEntryFunc)OsWowWriteFlashTask;
    param.taskPrio = OS_TSK_PRIORITY_LOWEST - 1;
    param.name = "WowWriteFlashTask";
    param.stackSize = 0x2000;

    return PRT_TaskCreate(&g_writeFlashTaskId, &param);
}

/*
 * LiteOs OsStoreSystemInfoBeforeSuspend: invoked from the idle/powermgr path
 * when OsRunstopWowSysDoneFlagGet() == OS_STORE_SYSTEM. Core 0 (the imaging
 * core) triggers the write task; other cores suspend directly.
 */
void OsRunstopStoreSystemInfoBeforeSuspend(void)
{
    U32 cpuid = OsGetCoreID();
    if (cpuid != 0) {
        OsRunstopSuspend();
    } else {
        (void)PRT_SemPost(g_writeFlashSem);
    }
}

/*
 * LiteOs OsWaitImagingDone / LOS_MakeImage: mark the system as resuming-from-image
 * and request imaging; block until OsRunstopSuspend signals completion.
 */
U32 OsRunstopMakeImage(void)
{
    U32 ret;

    if (!g_runstopInited) {
        return RUNSTOP_FAIL;
    }

    g_lowpowerResumeFromImg = LOWPOWER_RUNSTOP_RESET;
    g_lowpowerOtherCoreResume = 0;
    g_sysDoneFlag[OsGetCoreID()] = OS_STORE_SYSTEM;

    (void)PRT_SemPend(g_suspendResumeSem, OS_WAIT_FOREVER);
    ret = g_runstopResult;

    g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;
    return ret;
}

U32 OsRunstopStorePending(void)
{
    U32 cpuid = OsGetCoreID();
    g_sysDoneFlag[cpuid] = OS_STORE_SYSTEM;
    return RUNSTOP_OK;
}

/*
 * LiteOs OsSystemWakeup: restore the persisted image and CPU context. Called
 * from the power-manager reset handler when g_lowpowerResumeFromImg is set.
 */
void OsRunstopSystemWakeup(void)
{
    if (!IsImageResume()) {
        return;
    }

    if ((g_storageOps != NULL) && (g_storageOps->restoreImage != NULL)) {
        g_storageOps->restoreImage();
    }

    g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;
    g_lowpowerOtherCoreResume = 1;

    OsLowpowerRestoreRegister();
}

U32 OsRunstopWowSysDoneFlagGet(void)
{
    U32 cpuid = OsGetCoreID();
    if (cpuid >= OS_MAX_CORE_NUM) {
        return OS_NO_STORE_SYSTEM;
    }
    return g_sysDoneFlag[cpuid];
}

U32 OsRunstopInit(void)
{
    U32 ret;

    OsRunstopStateInit();

    ret = OsRunstopBackendInit();
    if (ret != RUNSTOP_OK) {
        PRT_Printf("\r\n [RUNSTOP] backend init failed: %u\n", ret);
        return ret;
    }

    if (OsRunstopResumeCheck() == RUNSTOP_RESUME) {
        /* Real resume path: restore the image and CPU context. */
        OsRunstopSystemWakeup();
    }

    ret = OsWowWriteFlashTaskCreate();
    if (ret != OS_OK) {
        PRT_Printf("\r\n [RUNSTOP] write-flash task create failed: %u\n", ret);
        return ret;
    }

    g_runstopInited = true;
    return RUNSTOP_OK;
}

#endif /* OS_OPTION_RUNSTOP */

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
 * Description: UniProton配置私有文件。
 */
#ifndef PRT_CONFIG_INTERNAL_H
#define PRT_CONFIG_INTERNAL_H

#include <stdint.h>
#include "prt_config.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "prt_sem.h"
#include "prt_tick.h"
#include "prt_exc.h"
#include "prt_cpup.h"
#include "prt_mem.h"
#include "prt_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define WORD_PACK(val) (((val) << 24) | ((val) << 16) | ((val) << 8) | (val))

/* UniProton模块注册函数的声明 */
extern U32 OsFscMemInit(uintptr_t addr, U32 size);
extern U32 OsSysRegister(struct SysModInfo *modInfo);
extern U32 OsTickRegister(struct TickModInfo *modInfo);
extern U32 OsTskRegister(struct TskModInfo *modInfo);
extern U32 OsCpupRegister(struct CpupModInfo *modInfo);
extern U32 OsSemRegister(const struct SemModInfo *modInfo);
extern U32 OsHookRegister(struct HookModInfo *modInfo);

extern U32 OsHwiConfigInit(void);
extern U32 OsTickConfigInit(void);
extern U32 OsTskInit(void);
extern U32 OsCpupInit(void);
extern void OsCpupWarnInit(void);
extern U32 OsExcConfigInit(void);
extern U32 OsSemInit(void);
extern U32 OsHookConfigInit(void);

/* UniProton系统启动相关函数的声明 */
extern void OsHwInit(void);
extern U32 OsActivate(void);
extern U32 OsTickStart(void);
extern U32 PRT_HardDrvInit(void);
/* Notes: PRT_HardBootInit接口在栈保护支持随机数设置场景下必须在bss初始化后调用 */
extern void PRT_HardBootInit(void);
extern U32 PRT_AppInit(void);

extern U32 OsQueueRegister(U16 maxQueue);
extern U32 OsQueueConfigInit(void);

#if (OS_INCLUDE_TICK_SWTMER == YES)
extern U32 OsSwTmrInit(U32 maxTimerNum);
#endif

enum OsinitPhaseId {
    OS_REGISTER_ID = 0,
    OS_INIT_ID,
    OS_MOUDLE_CONFIG
};
typedef U32 (*ConfigInitFunc)(void);
struct OsModuleConfigInfo {
    enum MoudleId moudleId;
    ConfigInitFunc moudleConfigFunc[OS_MOUDLE_CONFIG];
};
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_CONFIG_INTERNAL_H */

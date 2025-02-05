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
 * Create: 2024-03-08
 * Description: Perf
 */

#ifndef PRT_PERF_PMU_H
#define PRT_PERF_PMU_H

#include "include/prt_perf_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct {
    Pmu pmu;
    int canDivided;
    U32 cntDivided;
    void (*enable)(Event *event);
    void (*disable)(Event *event);
    void (*start)(void);
    void (*stop)(void);
    void (*clear)(void);
    void (*setPeriod)(Event *event);
    uintptr_t (*readCnt)(Event *event);
    U32 (*mapEvent)(U32 eventType, U32 reverse);
} HwPmu;

typedef struct {
    Pmu pmu;
    bool enable;
} SwPmu;

#define PRT_OFF_SET_OF(type, member) ((uintptr_t)&((type *)0)->member)
#define PRT_DL_LIST_ENTRY(item, type, member) \
    ((type *)(void *)((char *)(item) - PRT_OFF_SET_OF(type, member)))
#define GET_HW_PMU(item)                    PRT_DL_LIST_ENTRY(item, HwPmu, pmu)

#define CCNT_FULL                           0xFFFFFFFF
#define CCNT_PERIOD_LOWER_BOUND             0x00000000
#define CCNT_PERIOD_UPPER_BOUND             0xFFFFFF00
#define PERIOD_CALC(p)                      (CCNT_FULL - (p))
#define VALID_PERIOD(p)                     ((PERIOD_CALC(p) > CCNT_PERIOD_LOWER_BOUND) \
                                            && (PERIOD_CALC(p) < CCNT_PERIOD_UPPER_BOUND))

#define PERF_HW_INVAILD_EVENT_TYPE          0xFFFFFFFF

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

extern U32 OsPerfPmuRegister(Pmu *pmu);
extern void OsPerfPmuRm(U32 type);
extern Pmu *OsPerfPmuGet(U32 type);

extern U32 OsHwPmuInit(void);
extern U32 OsPerfHwInit(HwPmu *hwPmu);
extern U32 OsSwPmuInit(void);
extern U32 OsTimedPmuInit(void);

extern U32 OsGetPmuCounter0(void);
extern U32 OsGetPmuMaxCounter(void);
extern U32 OsGetPmuCycleCounter(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* PRT_PERF_PMU_H */

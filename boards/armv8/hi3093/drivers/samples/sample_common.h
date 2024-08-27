/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include "prt_hwi.h"

#ifndef OS_EMBED_ASM
#define OS_EMBED_ASM __asm__ __volatile__
#endif

#ifndef OS_DWORD_BIT_NUM
#define OS_DWORD_BIT_NUM 64
#endif

extern void OsGicSetTargetId(unsigned int intId, unsigned int targetId);

static inline void sample_hwi_affinity_set(unsigned int hwi_num, unsigned int core_mask)
{
    unsigned int target_core;
    unsigned int result;
    unsigned int rev = core_mask - 1;
    OS_EMBED_ASM("EOR %0, %1, %2" : "=r"(result) : "r"(rev), "r"(core_mask));
    OS_EMBED_ASM("CLZ %0, %1" : "=r"(rev) : "r"(result));
    target_core = OS_DWORD_BIT_NUM - rev - 1;
    OsGicSetTargetId(hwi_num, target_core);
}

void sample_prepare(void);
int app_main(void);

#define SAMPLE_CORE0    0
#define SAMPLE_CORE1    1
#define SAMPLE_CORE2    2
#define SAMPLE_CORE3    3

#endif /* __SAMPLE_COMMON_H__ */
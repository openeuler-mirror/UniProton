/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit allocator external interface for UniProton memory dispatch.
 */
#ifndef PRT_BESTFITMEM_EXTERNAL_H
#define PRT_BESTFITMEM_EXTERNAL_H

#include "prt_typedef.h"

extern U32 OsBestfitMemInit(uintptr_t addr, U32 size);
extern U32 OsBestfitMemFree(void *addr);

#endif /* PRT_BESTFITMEM_EXTERNAL_H */

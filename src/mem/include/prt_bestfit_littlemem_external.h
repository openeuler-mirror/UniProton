/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit_little memory allocator external interfaces.
 */
#ifndef PRT_BESTFIT_LITTLEMEM_EXTERNAL_H
#define PRT_BESTFIT_LITTLEMEM_EXTERNAL_H

#include "prt_mem_external.h"

extern U32 OsBestfitLittleMemInit(uintptr_t addr, U32 size);
extern U32 OsBestfitLittleMemFree(void *addr);

#endif /* PRT_BESTFIT_LITTLEMEM_EXTERNAL_H */

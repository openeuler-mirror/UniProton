/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
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
 * Description: 内存基本功能的C文件。
 */
#include "prt_mem_internal.h"
#include "prt_perf.h"
#include "prt_fscmem_external.h"
#include "prt_tlsfmem_external.h"

/*
 * 内存算法公共初始化分派：由 OsMemConfigReg 调用，根据 defconfig 选定的
 * 算法宏调用对应算法自己的初始化实现。FSC/TLSF 各自实现 OsFscMemInit/
 * OsTlsfMemInit 并填充 g_memArithAPI 分发表，config 层只感知本公共接口。
 */
OS_SEC_TEXT U32 OsMemInit(uintptr_t addr, U32 size)
{
#if defined(OS_MEM_ARITH_TLSF)
    return OsTlsfMemInit(addr, size);
#else
    return OsFscMemInit(addr, size);
#endif
}

OS_SEC_TEXT void *PRT_MemAlloc(U32 mid, U8 ptNo, U32 size)
{
#if defined(OS_OPTION_PERF) && defined(OS_OPTION_PERF_SW_PMU)
    PRT_PERF(MEM_ALLOC);
#endif
    void *addr;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    addr = g_memArithAPI.alloc(mid, ptNo, size);
    PRT_HwiRestore(intSave);

    return addr;
}

OS_SEC_TEXT void *PRT_MemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow)
{
    void *addr;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    addr = g_memArithAPI.allocAlign(mid, ptNo, size, alignPow);
    PRT_HwiRestore(intSave);

    return addr;
}

OS_SEC_TEXT U32 PRT_MemFree(U32 mid, void *addr)
{
    U32 ret;
    uintptr_t intSave;

    (void)mid;
    intSave = PRT_HwiLock();
    ret = g_memArithAPI.free(addr);
    PRT_HwiRestore(intSave);

    return ret;
}

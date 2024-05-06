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
 * Create: 2024-04-10
 * Description: 动态加载重定向处理模块
 */
#include "prt_dynmodule_internal.h"

static U32 OsDynModuleRelocateGlobDat(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo)
{
    *(Elf_Addr *)relocAddr = relocInfo.symAddr;
    return OS_MODULE_OK;
}

static U32 OsDynModuleRelocateJumpSlot(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo)
{
    *(Elf_Addr *)relocAddr = relocInfo.symAddr;
    return OS_MODULE_OK;
}

static U32 OsDynModuleRelocateRelative(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo)
{
    *(Elf_Addr *)relocAddr = relocInfo.symAddr + *(Elf_Addr *)relocAddr;
    return OS_MODULE_OK;
}

const struct OsDynModuleRelocateMap g_relocateMap[] = {
    { R_X86_64_GLOB_DAT, OsDynModuleRelocateGlobDat },
    { R_X86_64_JUMP_SLOT, OsDynModuleRelocateJumpSlot },
    { R_X86_64_RELATIVE, OsDynModuleRelocateRelative },
};

U32 OsDynModuleRelocate(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo)
{
    U32 i;
    size_t num = sizeof(g_relocateMap) / sizeof(g_relocateMap[0]);
    for (i = 0; i < num; i++) {
        if (g_relocateMap[i].relType == relocInfo.relocType) {
            return g_relocateMap[i].relFunc(relocAddr, relocInfo);
        }
    }
    return OS_MODULE_ERRNO_RELOCATE_INVALID_TYPE;
}

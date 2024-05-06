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
 * Description: 动态加载符号表
 */
#include "prt_dynmodule_internal.h"
#include "prt_dynamic_module.h"
#include "prt_mem.h"

U64 OsDynModuleFindSymFromOs(const char *symName)
{
    extern int os_symtab_start;
    extern int os_symtab_end;
    struct DynModuleSymTab *symTabStart = (struct DynModuleSymTab *)&os_symtab_start;
    struct DynModuleSymTab *symTabEnd = (struct DynModuleSymTab *)&os_symtab_end;
    struct DynModuleSymTab *symTabIndex = NULL;
    if (symName == NULL) {
        return 0;
    }

    for (symTabIndex = symTabStart; symTabIndex < symTabEnd; symTabIndex++) {
        if (symTabIndex->name == NULL) {
            continue;
        }
        if (strcmp(symName, symTabIndex->name) == 0) {
            return (U64)(symTabIndex->addr);
        }
    }
    return OS_MODULE_OK;
}

/* 在这里使用OS_SYMBOL_EXPORT来导出动态加载需要的OS的符号表，否则找不到符号表，加载失败 */
OS_SYMBOL_EXPORT(PRT_MemAlloc);

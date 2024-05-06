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
 * Description: 动态模块对外头文件, DL动态加载(dynamic loader), DP动态补丁(dynamic patch)
 */
#ifndef PRT_LP_H
#define PRT_LP_H

#include "prt_typedef.h"

struct DynModuleSymTab
{
    void *addr;                             // 符号地址
    char *name;                             // 符号名称
};

#define OS_SECTION(info) __attribute__((section(info)))
#define OS_SYMBOL_EXPORT(symbol) \
const char __dynModule_##symbol##_name[] OS_SECTION(".test") = { #symbol }; \
const struct DynModuleSymTab __dynModule_##symbol OS_SECTION(".OsSymTab") = { (void *)&symbol, __dynModule_##symbol##_name };

/**
 * 动态加载模块函数
 * 
 * 本函数用于加载指定的ELF文件，并对加载成功的模块进行初始化。
 * 
 * @param moduleFile 指向要加载的模块文件路径的指针。
 * @param mode 加载模式，具体模式的定义根据实际系统而定，当前仅支持立即绑定。
 * @return 成功返回加载的模块信息单元的指针，失败返回NULL。
 */
void* OsDynModuleLoad(const char *moduleFile, S32 mode);

/**
 * 在动态模块中查找指定的符号。
 * 
 * @param handle 动态模块加载后的句柄，用于标识一个动态模块。
 * @param symbol 需要查找的符号名称。
 * @return 如果找到指定的符号，则返回该符号的地址；如果没有找到，则返回NULL。
 */
void* OsDynModuleFind(void *handle, const char *symbol);

/**
 * 动态模块卸载函数
 *
 * @param handle 指向动态模块单元信息结构体的指针。该结构体包含了动态加载模块的详细信息。
 */
void OsDynModuleUnload(void *handle);

/**
 * 动态模块加载错误原因查询，有错误需要尽快查询，后续错误会覆盖前面的错误
 *
 * @return 返回错误原因字符串。
 */
const char *OsDynModuleGetError(void);

#endif

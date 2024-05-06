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
 * Description: 动态模块内部头文件
 */
#ifndef PRT_DYNMODULE_INTERNAL_H
#define PRT_DYNMODULE_INTERNAL_H

#include <elf.h>
#include <sys/stat.h>
#include "prt_typedef.h"
#include "prt_cpu_external.h"

#define OS_MODULE_ERROR_STR_LEN           512
#define OS_MODULE_ALIGN_LEN               4096

#define OS_ELF32_R_SYM(info)              ((info) >> 8)            /* r_info的高24位表示重定位入口的符号在符号表中的下标 */
#define OS_ELF64_R_SYM(info)              ((info) >> 32)           /* r_info的高32位表示重定位入口的符号在符号表中的下标 */
#define OS_ELF32_R_TYPE(info)             ((info) & 0xff)
#define OS_ELF64_R_TYPE(info)             ((info) & 0xffffffff)
#define ELF_ST_BIND(info)                 ((info) >> 4)            /* 获取符号的绑定类型 */
#define ELF_ST_TYPE(info)                 ((info) & 0xf)           /* 获取符号的类型 */

#if defined(OS_ARCH_CPU64)
typedef Elf64_Ehdr                        Elf_Ehdr;
typedef Elf64_Shdr                        Elf_Shdr;
typedef Elf64_Phdr                        Elf_Phdr;
typedef Elf64_Rel                         Elf_Rel;
typedef Elf64_Rela                        Elf_Rela;
typedef Elf64_Sym                         Elf_Sym;
typedef Elf64_Xword                       Elf_Word;
typedef Elf64_Sxword                      Elf_Sword;
typedef Elf64_Addr                        Elf_Addr;
#define ELF_R_SYM                         OS_ELF64_R_SYM
#define ELF_R_TYPE                        OS_ELF64_R_TYPE
#else
typedef Elf32_Ehdr                        Elf_Ehdr;
typedef Elf32_Shdr                        Elf_Shdr;
typedef Elf32_Phdr                        Elf_Phdr;
typedef Elf32_Rel                         Elf_Rel;
typedef Elf32_Rela                        Elf_Rela;
typedef Elf32_Sym                         Elf_Sym;
typedef Elf32_Word                        Elf_Word;
typedef Elf32_Sword                       Elf_Sword;
typedef Elf32_Addr                        Elf_Addr;
#define ELF_R_SYM                         OS_ELF32_R_SYM
#define ELF_R_TYPE                        OS_ELF32_R_TYPE
#endif

#define OS_MAX_MODULE_NUM                  5      // 最大可以加载5个模块

enum OS_MODULE_ERRNO_E {
    OS_MODULE_OK = 0,                             // 成功
    OS_MODULE_ERRNO_ELF_HEAD_LEN,                 // ELF头长度不正确
    OS_MODULE_ERRNO_ELF_HEAD_MAGIC,               // ELF头魔数字不正确
    OS_MODULE_ERRNO_ELF_HEAD_CLASS,               // ELF头class不正确
    OS_MODULE_ERRNO_ELF_HEAD_SHENT_SIZE,          // ELF头section header table size不正确
    OS_MODULE_ERRNO_ELF_HEAD_SHNUM,               // ELF头section header table数量不正确
    OS_MODULE_ERRNO_ELF_HEAD_MACHINE,             // ELF头machine不正确
    OS_MODULE_ERRNO_ELF_HEAD_TYPE,                // ELF头type不正确
    OS_MODULE_ERRNO_UNIT_FULL,                    // 模块单元已满
    OS_MODULE_ERRNO_PTLOAD_SIZE,                  // PT_LOAD段大小不正确
    OS_MODULE_ERRNO_MEMORY_ALLOC,                 // 内存分配失败
    OS_MODULE_ERRNO_MEMCOPY,                      // 内存拷贝失败
    OS_MODULE_ERRNO_OS_SYM,                       // 获取OS的符号失败
    OS_MODULE_ERRNO_RELOCATE_INVALID_TYPE,        // 重定位类型错误
    OS_MODULE_ERRNO_RELOCATE,                     // 重定位失败
    OS_MODULE_ERRNO_DYNSYM_SECTION_NOT_FOUND,     // 找不到.dynsym段
    OS_MODULE_ERRNO_RELOCATE_SYM_VAL_OVERFLOW,    // 重定位符号值溢出
    OS_MODULE_ERRNO_NO_GLOBAL_FUNC,               // 没有全局函数
    OS_MODULE_ERRNO_FILE_CHECK_FAILED,            // 模块文件校验失败
    OS_MODULE_ERRNO_FILE_OPEN,                    // 模块文件打开失败
    OS_MODULE_ERRNO_FILE_READ,                    // 模块文件读取失败
};

enum ModuleUnitSate {
    MODULE_UNIT_FREE = 0,
    MODULE_UNIT_INIT,
    MODULE_UNIT_ACTIVE,
};

struct DynModuleUnitInfo
{
    U8 unitNo;                                   // 模块单元编号
    U16 symNum;                                  // 符号数量
    enum ModuleUnitSate state;                   // 模块单元状态
    U64 loadSegStartAddr;                        // 要加载的段起始地址
    U64 loadSegEndAddr;                          // 要加载的段结束地址
	dev_t stDev;                                 // 模块所在设备
	ino_t stIno;                                 // 模块所在inode
    struct DynModuleSymTab *symTab;              // 符号表
    uint8_t *loadSegMem;                         // 加载段内存
    char *moduleStr;                             // 模块字符串
    char *error;                                 // 错误信息
};

struct OsDynModuleRelocInfo {
    Elf_Word relocType;
    Elf_Word shType;
    Elf_Addr symAddr;
    uintptr_t reloc;
};

/* 通用重定向回调函数，适用于所有平台所有重定向类型 */
typedef U32 (*OsDynModuleRelocateFunc)(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo);

struct OsDynModuleRelocateMap {
    U32 relType;
    OsDynModuleRelocateFunc relFunc;
};


#if defined(OS_OPTION_SMP)
extern volatile uintptr_t g_osModuleIntLock;
OS_SEC_ALW_INLINE INLINE uintptr_t OsModuleIntLock(void)
{
    uintptr_t intSave = OsIntLock();
    OsSplLock(&g_osModuleIntLock);
    return intSave;
}

OS_SEC_ALW_INLINE INLINE void OsModuleIntUnlock(uintptr_t intSave)
{
    OsSplUnlock(&g_osModuleIntLock);
    OsIntRestore(intSave);
}
#else
OS_SEC_ALW_INLINE INLINE uintptr_t OsModuleIntLock(void)
{
    return OsIntLock();
}

OS_SEC_ALW_INLINE INLINE void OsModuleIntUnlock(uintptr_t intSave)
{
    OsIntRestore(intSave);
}
#endif

U32 OsDynModuleRelocate(Elf_Addr relocAddr, struct OsDynModuleRelocInfo relocInfo);
U64 OsDynModuleFindSymFromOs(const char *symName);
#endif

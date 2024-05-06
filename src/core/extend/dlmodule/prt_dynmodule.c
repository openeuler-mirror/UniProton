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
 * Description: 动态加载处理模块
 */
#include "prt_dynamic_module.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include "prt_dynmodule_internal.h"
#include "prt_module.h"
#include "prt_mem.h"
#include "securec.h"

#ifndef OS_ARCH_X86_64
extern void os_asm_invalidate_dcache_all(void);
extern void os_asm_invalidate_icache_all(void);
#endif

#if defined(OS_OPTION_SMP)
OS_SEC_BSS volatile uintptr_t g_osModuleIntLock;
#endif

struct DynModuleUnitInfo *g_dynModuleInfoPool[OS_MAX_MODULE_NUM] = {0};

char *g_dynModuleErrorStr = NULL;

/**
 * @brief 设置动态模块的错误信息。
 * 
 * 该函数使用格式化字符串和可变参数来生成错误信息，不建议错误信息长度不能超过OS_MODULE_ERROR_STR_LEN
 * 每次只能保存一次错误信息，需要及时获取，后面会覆盖。
 * 当前仅支持单核，等待多核功能支持后需要考虑多核问题。
 * 
 * @param fmt 格式化字符串，用于错误信息的格式化。
 * @param ... 可变参数，与格式化字符串对应，用于填充错误信息。
 */
static void OsDynModuleSetError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (g_dynModuleErrorStr == NULL) {
        g_dynModuleErrorStr = (char *)PRT_MemAlloc(OS_MID_DYNAMIC, OS_MEM_DEFAULT_FSC_PT, OS_MODULE_ERROR_STR_LEN);
        if (g_dynModuleErrorStr == NULL) {
            va_end(ap);
            return;
        }
    }
    (void)memset_s(g_dynModuleErrorStr, OS_MODULE_ERROR_STR_LEN, 0, OS_MODULE_ERROR_STR_LEN);
    int ret = vsnprintf_s(g_dynModuleErrorStr, OS_MODULE_ERROR_STR_LEN, OS_MODULE_ERROR_STR_LEN, fmt, ap);
    if (ret == -1) {
        (void)PRT_MemFree(OS_MID_DYNAMIC, g_dynModuleErrorStr);
        g_dynModuleErrorStr = NULL;
	    va_end(ap);
        return;
    }
	va_end(ap);
}

/**
 * 从指定的模块文件中获取文件内容
 * 
 * @param moduleFile 指向要读取的模块文件名的字符指针。
 * @return 返回一个指向包含文件内容的字符数组的指针；如果失败，则返回NULL。
 */
static U32 OsDynModuleGetFileContent(const char *moduleFile, char **buf)
{
    int fd = -1;
    unsigned long length;
    char *moduleStr = NULL;
    fd = open(moduleFile, O_RDONLY);
    if (fd < 0) {
        return OS_MODULE_ERRNO_FILE_OPEN;
    }
    length = lseek(fd, 0, SEEK_END); 
    lseek(fd, 0, SEEK_SET);
    moduleStr = (char *)PRT_MemAlloc(OS_MID_DYNAMIC, OS_MEM_DEFAULT_FSC_PT, length + 1);
    if (moduleStr == NULL) {
        close(fd);
        return OS_MODULE_ERRNO_MEMORY_ALLOC;
    }
    (void)memset_s(moduleStr, length + 1, 0, length + 1);
    if(read(fd, moduleStr, length) != length) {
        (void)PRT_MemFree(OS_MID_DYNAMIC, moduleStr);
        close(fd);
        return OS_MODULE_ERRNO_FILE_READ;
    }
    close(fd);
    *buf = moduleStr;
    return OS_MODULE_OK;
}

/**
 * 动态模块加载时，检查ELF文件格式是否正确。
 * 
 * @param lfInfo 指向ELF头信息的指针。
 * @return 成功返回0，失败返回错误码
 */
static U32 OsDynModuleCheckElf(Elf_Ehdr *elfInfo)
{
    /* 魔术字校验 */
    if ((elfInfo->e_ident[EI_MAG0] != ELFMAG0) || (elfInfo->e_ident[EI_MAG1] != ELFMAG1) ||
        (elfInfo->e_ident[EI_MAG2] != ELFMAG2) || (elfInfo->e_ident[EI_MAG3] != ELFMAG3)) {
        return OS_MODULE_ERRNO_ELF_HEAD_MAGIC;
    }
    /* ELF文件类型校验 */
    if ((elfInfo->e_ident[EI_CLASS] != ELFCLASS32) && (elfInfo->e_ident[EI_CLASS] != ELFCLASS64)) {
        return OS_MODULE_ERRNO_ELF_HEAD_CLASS;
    }
    /* ELF头文件大小校验 */
    if (elfInfo->e_ehsize != sizeof(Elf_Ehdr)) {
        return OS_MODULE_ERRNO_ELF_HEAD_LEN;
    }
    /* 判断节区表大小 */
    if (elfInfo->e_shentsize != sizeof(Elf_Shdr)) {
        return OS_MODULE_ERRNO_ELF_HEAD_SHENT_SIZE;
    }
    /* 检查节区个数 */
    if (elfInfo->e_shnum == 0) {
        return OS_MODULE_ERRNO_ELF_HEAD_SHNUM;
    }
    /* 当前仅支持x86和ARM64架构 */
    if ((elfInfo->e_machine != EM_X86_64) && (elfInfo->e_machine != EM_AARCH64)) {
        return OS_MODULE_ERRNO_ELF_HEAD_MACHINE;
    }
    /* 当前仅支持DYN类型 */
    if (elfInfo->e_type != ET_DYN) {
        return OS_MODULE_ERRNO_ELF_HEAD_TYPE;
    }
    return OS_MODULE_OK;
}

/**
 * 动态模块单元分配函数
 * 本函数用于从动态模块单元信息池中分配一个未使用的单元。如果找到可用单元，则将其状态设置为初始化，并返回其信息指针。
 * 如果所有单元都已被使用，则函数返回错误码。
 *
 * @param dynModuleUnitInfo 指向动态模块单元信息指针的双指针，用于返回分配到的单元信息的地址。
 * @return 成功返回0，如果所有单元都已分配
 */
static U32 OsDynModuleUnitAlloc(struct DynModuleUnitInfo **dynModuleUnitInfo)
{
    U32 i;
    struct DynModuleUnitInfo *unitInfo = NULL;
    uintptr_t intSave = OsModuleIntLock();

    for (i = 0; i < OS_MAX_MODULE_NUM; i++) {
        if (g_dynModuleInfoPool[i] != NULL) {
            continue;
        }
        unitInfo = (struct DynModuleUnitInfo *)PRT_MemAlloc(OS_MID_DYNAMIC, 
            OS_MEM_DEFAULT_FSC_PT, sizeof(struct DynModuleUnitInfo));
        if (unitInfo == NULL) {
            OsModuleIntUnlock(intSave);
            return OS_MODULE_ERRNO_MEMORY_ALLOC;
        }
        (void)memset_s(unitInfo, sizeof(struct DynModuleUnitInfo), 0, sizeof(struct DynModuleUnitInfo));
        break;
    }

    if (unitInfo == NULL) {
        OsModuleIntUnlock(intSave);
        return OS_MODULE_ERRNO_UNIT_FULL;
    }

    g_dynModuleInfoPool[i] = unitInfo;
    unitInfo->unitNo = i;
    unitInfo->state = MODULE_UNIT_FREE;
    *dynModuleUnitInfo = unitInfo;
    OsModuleIntUnlock(intSave);
    return OS_MODULE_OK;
}

/**
 * 为动态模块分配内存以加载段。
 * 
 * 该函数遍历ELF文件中的程序头（Program Headers），找到所有类型为PT_LOAD的段（segment），
 * 并为这些段分配足够的内存。它记录段的起始地址和结束地址，并实际分配内存以供后续加载模块使用。
 * 
 * @param unitInfo 指向包含模块信息的结构体的指针。该结构体应包含模块的字符串表示（即ELF文件的起始地址）。
 * @return 成功返回0，失败返回特定错误码，指示无法分配内存或PT_LOAD段的大小为0。
 */
static U32 OsDynModuleAllocMemForLoadSeg(struct DynModuleUnitInfo *unitInfo)
{
    U32 i;
    Elf_Ehdr *elfInfo = (Elf_Ehdr *)(unitInfo->moduleStr);
    Elf_Phdr *phdr = (Elf_Phdr *)(unitInfo->moduleStr + elfInfo->e_phoff);
    bool flag = false;
    for (i = 0; i < elfInfo->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) {
            continue;
        }
        if (phdr[i].p_filesz > phdr[i].p_memsz) {
            return OS_MODULE_ERRNO_PTLOAD_SIZE;
        }
        if (!flag) {
            unitInfo->loadSegStartAddr = phdr[i].p_vaddr;
            flag = true;
        }
        unitInfo->loadSegEndAddr = phdr[i].p_vaddr + phdr[i].p_memsz;
    }
    U32 size = unitInfo->loadSegEndAddr - unitInfo->loadSegStartAddr;
    if (size == 0) {
        return OS_MODULE_ERRNO_PTLOAD_SIZE;
    }
    size = ALIGN(size, OS_MODULE_ALIGN_LEN);
    unitInfo->loadSegMem = (uint8_t *)PRT_MemAllocAlign(OS_MID_DYNAMIC, OS_MEM_DEFAULT_FSC_PT, size, MEM_ADDR_ALIGN_4K);
    if (unitInfo->loadSegMem == NULL) {
        return OS_MODULE_ERRNO_MEMORY_ALLOC;
    }
    (void)memset_s(unitInfo->loadSegMem, size, 0, size);
    return OS_MODULE_OK;
}

/**
 * 从动态模块中获取加载段数据
 * 
 * 本函数用于从动态模块的内存表示中提取出所有需要加载到内存的段数据，并将这些数据复制到
 * 指定的内存区域。该操作是基于ELF格式的动态模块。
 *
 * @param unitInfo 指向动态模块单元信息的指针。单元信息包含了模块的字符串起始地址、加载段的内存起始地址等。
 * @return 函数成功执行时返回0，若复制过程中发生错误则返回OS_MODULE_ERRNO_MEMCOPY。
 */
static U32 OsDynModuleGetLoadSegData(struct DynModuleUnitInfo *unitInfo)
{
    U32 i, ret;
    Elf_Ehdr *elfInfo = (Elf_Ehdr *)(unitInfo->moduleStr);
    Elf_Phdr *phdr = (Elf_Phdr *)(unitInfo->moduleStr + elfInfo->e_phoff);
    for (i = 0; i < elfInfo->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) {
            continue;
        }
        /* 前面申请内存的时候已经初始化0了，这里拷贝的大小使用p_filesz而不是用p_memsz的原因就是memsz-filesz这一块是bss，需要清0 */
        ret = memcpy_s(unitInfo->loadSegMem + phdr[i].p_vaddr - unitInfo->loadSegStartAddr,
            phdr[i].p_filesz, unitInfo->moduleStr + phdr[i].p_offset, phdr[i].p_filesz);
        if (ret != EOK) {
            return OS_MODULE_ERRNO_MEMCOPY;
        }
    }
#ifndef OS_ARCH_X86_64
    os_asm_invalidate_dcache_all();
#endif
    return OS_MODULE_OK;
}

static U32 OsDynModuleRelocatePrepare(struct DynModuleUnitInfo *unitInfo, uintptr_t reloc,
    Elf_Sym *symTab, uint8_t *strTab, Elf_Shdr *shdr)
{
    U32 ret;
    Elf_Addr offset;
    Elf_Sym *sym;
    Elf_Sword addend = 0;
    Elf_Addr relocAddr;
    Elf_Addr symAddr;
    Elf_Word type;
    struct OsDynModuleRelocInfo relocInfo = {0};
    if (shdr->sh_type == SHT_RELA) {
        Elf_Rela *rela = (Elf_Rela *)(reloc);
        offset = rela->r_offset;
        sym = symTab + (uintptr_t)ELF_R_SYM(rela->r_info); /* 获取重定位符号 */
        type = ELF_R_TYPE(rela->r_info);
    } else {
        Elf_Rel *rel = (Elf_Rel *)(reloc);
        offset = rel->r_offset;
        sym = symTab + (uintptr_t)ELF_R_SYM(rel->r_info);
        type = ELF_R_TYPE(rel->r_info);
    }
    relocAddr = (Elf_Addr)(unitInfo->loadSegMem + offset - unitInfo->loadSegStartAddr);
    if ((sym->st_shndx != SHT_NULL) || (ELF_ST_BIND(sym->st_info) == STB_LOCAL)) {
        symAddr = (Elf_Addr)(unitInfo->loadSegMem + sym->st_value - unitInfo->loadSegStartAddr);
    } else {
        symAddr = OsDynModuleFindSymFromOs(strTab + sym->st_name);
    }
    relocInfo.reloc = reloc;
    relocInfo.relocType = type;
    relocInfo.shType = shdr->sh_type;
    relocInfo.symAddr = symAddr;
    ret = OsDynModuleRelocate(relocAddr, relocInfo);
    if (ret != 0) {
        return ret;
    }
    return OS_MODULE_OK;
}

static U32 OsDynModuleProcessRelocationSection(struct DynModuleUnitInfo *unitInfo)
{
    U32 i, j, ret;
    Elf_Ehdr *elfInfo = (Elf_Ehdr *)(unitInfo->moduleStr);
    Elf_Shdr *shdr = (Elf_Shdr *)(unitInfo->moduleStr + elfInfo->e_shoff);
    uintptr_t reloc;
    for (i = 0; i < elfInfo->e_shnum; i++) {
        if ((shdr[i].sh_type != SHT_RELA) && (shdr[i].sh_type != SHT_REL)) {
            continue;
        }
        /* 获取要处理的节区表.rela.dyn或.rela.plt */
        reloc = (uintptr_t)(unitInfo->moduleStr + shdr[i].sh_offset);
        /* 通过sh_link获取到符号表节区.dynsym */
        Elf_Sym *symTab = (Elf_Sym *)(unitInfo->moduleStr + shdr[shdr[i].sh_link].sh_offset);
        /* 通过节区.dynsym的sh_link获取到符号名字字符串表节区.dynstr */
        uint8_t *strTab = (uint8_t *)(unitInfo->moduleStr + shdr[shdr[shdr[i].sh_link].sh_link].sh_offset);
        for (j = 0; j < shdr[i].sh_size / shdr[i].sh_entsize; j++, reloc += shdr[i].sh_entsize) {
            /* 从重定向表里获取每一个重定向表项reloc */
            ret = OsDynModuleRelocatePrepare(unitInfo, reloc, symTab, strTab, &shdr[i]);
            if (ret != OS_MODULE_OK) {
                return ret;
            }
        }
    }
#ifndef OS_ARCH_X86_64
    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
#endif
    return OS_MODULE_OK;
}

/**
 * 保存动态模块的符号表信息，供外部调用
 * 
 * 该函数负责解析动态模块的ELF文件格式，找到符号表（.dynsym），
 * 并从中提取出所有的全局函数符号，保存到模块的符号表结构体中。
 * 
 * @param unitInfo 指向动态模块单元信息的指针，包含了模块的字符串起始地址等信息。
 * @return 成功返回OS_MODULE_OK，否则返回相应的错误码。
 */
static U32 OsDynModuleSaveSymTab(struct DynModuleUnitInfo *unitInfo)
{
    U32 ret;
    U16 index, i, j = 0;
    Elf_Ehdr *elfInfo = (Elf_Ehdr *)(unitInfo->moduleStr);
    Elf_Shdr *shdr = (Elf_Shdr *)(unitInfo->moduleStr + elfInfo->e_shoff);
    uint8_t *shstrndx = (uint8_t *)(unitInfo->moduleStr + shdr[elfInfo->e_shstrndx].sh_offset);
    for (index = 0; index < elfInfo->e_shnum; index++) {
        if (strcmp((const char *)(shstrndx + shdr[index].sh_name), ".dynsym") == 0) {
            break;
        }
    }
    if (index == elfInfo->e_shnum) {
        return OS_MODULE_ERRNO_DYNSYM_SECTION_NOT_FOUND;
    }
    /* 获取节区.dynsym的信息，符号表节区，其结构为Elf_Sym */
    Elf_Sym *symTab = (Elf_Sym *)(unitInfo->moduleStr + shdr[index].sh_offset);
    /* 获取节区.dynstr的信息，符号名字字符串表节区，其结构为uint8_t */
    uint8_t *strTab = (uint8_t *)(unitInfo->moduleStr + shdr[shdr[index].sh_link].sh_offset);
    /* 计算节区有多少个全局函数 */
    for (i = 0; i < shdr[index].sh_size / shdr[index].sh_entsize; i++) {
        if ((ELF_ST_TYPE(symTab[i].st_info) == STT_FUNC) && (ELF_ST_BIND(symTab[i].st_info) == STB_GLOBAL)) {
            unitInfo->symNum++;
        }
    }
    if (unitInfo->symNum == 0) {
        return OS_MODULE_ERRNO_NO_GLOBAL_FUNC;
    }
    unitInfo->symTab = (struct DynModuleSymTab *)PRT_MemAlloc(OS_MID_DYNAMIC, OS_MEM_DEFAULT_FSC_PT,
        unitInfo->symNum * sizeof(struct DynModuleSymTab));
    if (unitInfo->symTab == NULL) {
        return OS_MODULE_ERRNO_MEMORY_ALLOC;
    }
    (void)memset_s(unitInfo->symTab, unitInfo->symNum * sizeof(struct DynModuleSymTab), 0,
        unitInfo->symNum * sizeof(struct DynModuleSymTab));
    /* 获取符号信息 */
    for (i = 0; i < shdr[index].sh_size / shdr[index].sh_entsize; i++) {
        if ((ELF_ST_TYPE(symTab[i].st_info) != STT_FUNC) || (ELF_ST_BIND(symTab[i].st_info) != STB_GLOBAL)) {
            continue;
        }
        /* 获取符号地址 */
        unitInfo->symTab[j].addr = (void *)(unitInfo->loadSegMem + symTab[i].st_value - unitInfo->loadSegStartAddr);
        size_t symNameLen = strlen(strTab + symTab[i].st_name) + 1;
        unitInfo->symTab[j].name = (char *)PRT_MemAlloc(OS_MID_DYNAMIC, OS_MEM_DEFAULT_FSC_PT, symNameLen);
        if (unitInfo->symTab[j].name == NULL) {
            ret =  OS_MODULE_ERRNO_MEMORY_ALLOC;
            goto FAIL_RET;
        }
        (void)memset_s(unitInfo->symTab[j].name, symNameLen, 0, symNameLen);
        errno_t err = strcpy_s(unitInfo->symTab[j].name, symNameLen, (const char *)(strTab + symTab[i].st_name));
        if (err != OS_MODULE_OK) {
            ret = OS_MODULE_ERRNO_MEMCOPY;
            goto FAIL_RET;
        }
        j++;
    }
    return OS_MODULE_OK;
FAIL_RET:
    for (i = 0; i < j; i++) {
        if (unitInfo->symTab[i].name != NULL) {
            (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->symTab[i].name);
            unitInfo->symTab[i].name = NULL;
        }
    }
    if (unitInfo->symTab != NULL) {
        (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->symTab);
        unitInfo->symTab = NULL;
    }
    return ret;
}

static U32 OsDynModuleUnitLoad(struct DynModuleUnitInfo *unitInfo, S32 mode)
{
    (void)mode;
    U32 ret;
    ret = OsDynModuleAllocMemForLoadSeg(unitInfo);
    if (ret != 0) {
        return ret;
    }
    ret = OsDynModuleGetLoadSegData(unitInfo);
    if (ret != 0) {
        return ret;
    }
    ret = OsDynModuleProcessRelocationSection(unitInfo);
    if (ret != 0) {
        return ret;
    }
    ret = OsDynModuleSaveSymTab(unitInfo);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

static void OsDynModuleUnitFree(struct DynModuleUnitInfo *unitInfo)
{
    U32 i;
    if (unitInfo == NULL) {
        return;
    }
    if (unitInfo->moduleStr != NULL) {
        (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->moduleStr);
        unitInfo->moduleStr = NULL;
    }
    if (unitInfo->symTab != NULL) {
        for (i = 0; i < unitInfo->symNum; i++) {
            if (unitInfo->symTab[i].name != NULL) {
                (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->symTab[i].name);
                unitInfo->symTab[i].name = NULL;
            }
        }
        (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->symTab);
        unitInfo->symTab = NULL;
    }
    if (unitInfo->loadSegMem != NULL) {
        (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->loadSegMem);
        unitInfo->loadSegMem = NULL;
    }
    g_dynModuleInfoPool[unitInfo->unitNo] = NULL;
    (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo);
    unitInfo = NULL;
}

static U32 OsDynModuleFileCheck(const char *moduleFile, struct stat *moduleFileSata, struct DynModuleUnitInfo **unitInfo)
{
    int result;
    U32 i;
    result = stat(moduleFile, moduleFileSata);
    if (result != 0) {
        return OS_MODULE_ERRNO_FILE_CHECK_FAILED;
    }
    for (i = 0; i < OS_MAX_MODULE_NUM; i++) {
        if (g_dynModuleInfoPool[i] == NULL) {
            continue;
        }
        if ((moduleFileSata->st_dev == g_dynModuleInfoPool[i]->stDev) &&
            (moduleFileSata->st_ino == g_dynModuleInfoPool[i]->stIno)) {
            *unitInfo = g_dynModuleInfoPool[i];
            break;
        }
    }
    return OS_MODULE_OK;
}

void* OsDynModuleLoad(const char *moduleFile, S32 mode)
{
    U32 ret;
    char *moduleStr = NULL;
    Elf_Ehdr *elfInfo = NULL;
    struct stat moduleFileSata;
    struct DynModuleUnitInfo *unitInfo = NULL;
    if (moduleFile == NULL) {
        OsDynModuleSetError("The file path is empty.");
        return NULL;
    }

    ret = OsDynModuleFileCheck(moduleFile, &moduleFileSata, &unitInfo);
    if (ret != OS_MODULE_OK) {
        OsDynModuleSetError("File check failed.");
        return NULL;
    }

    if (unitInfo != NULL) {
        if (unitInfo->state == MODULE_UNIT_INIT) {
            OsDynModuleSetError("The module is already loading, please try again later.");
            return NULL;
        }
        return unitInfo;
    }

    ret = OsDynModuleGetFileContent(moduleFile, &moduleStr);
    if ((ret != OS_MODULE_OK) || (moduleStr == NULL)) {
        OsDynModuleSetError("Get file content failed, ret:%u.", ret);
        return NULL;
    }

    elfInfo = (Elf_Ehdr *)moduleStr;
    ret = OsDynModuleCheckElf(elfInfo);
    if (ret != 0) {
        OsDynModuleSetError("ELF check failed, ret:%u.", ret);
        (void)PRT_MemFree(OS_MID_DYNAMIC, moduleStr);
        return NULL;
    }

    ret = OsDynModuleUnitAlloc(&unitInfo);
    if (ret != 0) {
        OsDynModuleSetError("Alloc memory failed for the dynamic module unit, ret:%u.", ret);
        (void)PRT_MemFree(OS_MID_DYNAMIC, moduleStr);
        return NULL;
    }
    unitInfo->moduleStr = moduleStr;
    unitInfo->stDev = moduleFileSata.st_dev;
    unitInfo->stIno = moduleFileSata.st_ino;
    unitInfo->state = MODULE_UNIT_INIT;

    ret = OsDynModuleUnitLoad(unitInfo, mode);
    if (ret != 0) {
        OsDynModuleSetError("Load the dynamic module unit failed, ret:%u.", ret);
        OsDynModuleUnitFree(unitInfo);
        return NULL;
    }

    (void)PRT_MemFree(OS_MID_DYNAMIC, unitInfo->moduleStr);
    unitInfo->moduleStr = NULL;
    unitInfo->state = MODULE_UNIT_ACTIVE;
    return (void *)unitInfo;
}

void* OsDynModuleFind(void *handle, const char *symbol)
{
    struct DynModuleUnitInfo *unitInfo = (struct DynModuleUnitInfo *)handle;
    if (unitInfo == NULL) {
        return NULL;
    }
    U16 i;
    for (i = 0; i < unitInfo->symNum; i++) {
        if (strcmp(unitInfo->symTab[i].name, symbol) == 0) {
            return unitInfo->symTab[i].addr;
        }
    }
    return NULL;
}

void OsDynModuleUnload(void *handle)
{
    struct DynModuleUnitInfo *unitInfo = (struct DynModuleUnitInfo *)handle;
    if (unitInfo == NULL) {
        return;
    }
    OsDynModuleUnitFree(unitInfo);
}

const char *OsDynModuleGetError(void)
{
    return (const char *)g_dynModuleErrorStr;
}

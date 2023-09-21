/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 *
 * Author: niutao2@huawei.com
 * Create: 2023-09-20
 * Description: arm64 gdb实现
 */

#include "prt_typedef.h"
#include "gdbstub.h"
#include "arch_interface.h"
#include "rsp_utils.h"

STUB_DATA static struct DbgRegDef g_dbgRegDef[DBG_MAX_REG_NUM] = {
    { "x0", 8, offsetof(struct PrtRegs, regs[0]) },
    { "x1", 8, offsetof(struct PrtRegs, regs[1]) },
    { "x2", 8, offsetof(struct PrtRegs, regs[2]) },
    { "x3", 8, offsetof(struct PrtRegs, regs[3]) },
    { "x4", 8, offsetof(struct PrtRegs, regs[4]) },
    { "x5", 8, offsetof(struct PrtRegs, regs[5]) },
    { "x6", 8, offsetof(struct PrtRegs, regs[6]) },
    { "x7", 8, offsetof(struct PrtRegs, regs[7]) },
    { "x8", 8, offsetof(struct PrtRegs, regs[8]) },
    { "x9", 8, offsetof(struct PrtRegs, regs[9]) },
    { "x10", 8, offsetof(struct PrtRegs, regs[10]) },
    { "x11", 8, offsetof(struct PrtRegs, regs[11]) },
    { "x12", 8, offsetof(struct PrtRegs, regs[12]) },
    { "x13", 8, offsetof(struct PrtRegs, regs[13]) },
    { "x14", 8, offsetof(struct PrtRegs, regs[14]) },
    { "x15", 8, offsetof(struct PrtRegs, regs[15]) },
    { "x16", 8, offsetof(struct PrtRegs, regs[16]) },
    { "x17", 8, offsetof(struct PrtRegs, regs[17]) },
    { "x18", 8, offsetof(struct PrtRegs, regs[18]) },
    { "x19", 8, offsetof(struct PrtRegs, regs[19]) },
    { "x20", 8, offsetof(struct PrtRegs, regs[20]) },
    { "x21", 8, offsetof(struct PrtRegs, regs[21]) },
    { "x22", 8, offsetof(struct PrtRegs, regs[22]) },
    { "x23", 8, offsetof(struct PrtRegs, regs[23]) },
    { "x24", 8, offsetof(struct PrtRegs, regs[24]) },
    { "x25", 8, offsetof(struct PrtRegs, regs[25]) },
    { "x26", 8, offsetof(struct PrtRegs, regs[26]) },
    { "x27", 8, offsetof(struct PrtRegs, regs[27]) },
    { "x28", 8, offsetof(struct PrtRegs, regs[28]) },
    { "x29", 8, offsetof(struct PrtRegs, regs[29]) },
    { "x30", 8, offsetof(struct PrtRegs, regs[30]) },
    { "sp", 8, offsetof(struct PrtRegs, sp) },
    { "pc", 8, offsetof(struct PrtRegs, pc) },
    /*
     * struct PrtRegs thinks PSTATE is 64-bits wide but gdb remote
     * protocol disagrees. Therefore we must extract only the lower
     * 32-bits. Look for the big comment in __asm__ /kgdb.h for more
     * detail.
     */
    { "pstate", 4, offsetof(struct PrtRegs, pstate) },
    { "v0", 16, -1 },
    { "v1", 16, -1 },
    { "v2", 16, -1 },
    { "v3", 16, -1 },
    { "v4", 16, -1 },
    { "v5", 16, -1 },
    { "v6", 16, -1 },
    { "v7", 16, -1 },
    { "v8", 16, -1 },
    { "v9", 16, -1 },
    { "v10", 16, -1 },
    { "v11", 16, -1 },
    { "v12", 16, -1 },
    { "v13", 16, -1 },
    { "v14", 16, -1 },
    { "v15", 16, -1 },
    { "v16", 16, -1 },
    { "v17", 16, -1 },
    { "v18", 16, -1 },
    { "v19", 16, -1 },
    { "v20", 16, -1 },
    { "v21", 16, -1 },
    { "v22", 16, -1 },
    { "v23", 16, -1 },
    { "v24", 16, -1 },
    { "v25", 16, -1 },
    { "v26", 16, -1 },
    { "v27", 16, -1 },
    { "v28", 16, -1 },
    { "v29", 16, -1 },
    { "v30", 16, -1 },
    { "v31", 16, -1 },
    { "fpsr", 4, -1 },
    { "fpcr", 4, -1 },
};

STUB_DATA static struct GdbCtx g_debugContext;
STUB_DATA static bool g_compiled_brk = false;

#define REGS (g_debugContext.regs.regs)
#define RBUF (g_debugContext.regs.rbuf)

/* mask/save/unmask/restore all exceptions, including interrupts. */
static inline void local_daif_mask(void)
{
    __asm__  volatile(
        "msr    daifset, #0xf"
        :
        :
        : "memory");
}

static inline void local_daif_clr_flags(uint32_t flags)
{
    __asm__  volatile(
        "msr    daifset, %0"
        :
        : "i"(flags)
        : "memory");
}

static inline unsigned long local_daif_save_flags(void)
{
    unsigned long flags;

    flags = read_sysreg(daif);

    return flags;
}

static inline unsigned long local_daif_save(void)
{
    unsigned long flags;

    flags = local_daif_save_flags();

    local_daif_mask();

    return flags;
}

static inline void local_daif_restore(unsigned long flags)
{
    write_sysreg(flags, daif);
}

STUB_TEXT static void mdscr_write(uint32_t mdscr)
{
    unsigned long flags;
    flags = local_daif_save();
    write_sysreg(mdscr, mdscr_el1);
    local_daif_restore(flags);
}

STUB_TEXT static uint32_t mdscr_read(void)
{
    return read_sysreg(mdscr_el1);
}

STUB_TEXT void OsGdbArchContinue(void)
{
    if (g_compiled_brk) {
        REGS.pc += BREAK_INSTR_SIZE;
        g_compiled_brk = false;
    }

    REGS.pstate &= ~DBG_SPSR_SS;
    REGS.pstate &= ~DBG_SPSR_D;

    mdscr_write(mdscr_read() & ~DBG_MDSCR_SS);
    mdscr_write(mdscr_read() & ~(DBG_MDSCR_KDE));
}

STUB_TEXT void OsGdbArchStep(void)
{
    if (g_compiled_brk) {
        REGS.pc += BREAK_INSTR_SIZE;
        g_compiled_brk = false;
    }

    REGS.pstate |= DBG_SPSR_SS;
    REGS.pstate &= ~DBG_SPSR_D;
    mdscr_write(mdscr_read() | DBG_MDSCR_SS);
    mdscr_write(mdscr_read() | DBG_MDSCR_KDE);
}

STUB_TEXT void OsGdbArchPrepare(void *stk)
{
    struct PrtRegs *orig = (struct PrtRegs *)stk;

    for (int i = 0; i < sizeof(orig->regs) / sizeof(orig->regs[0]); i++) {
        REGS.regs[i] = orig->regs[i];
    }
    
    REGS.sp = orig->sp;
    REGS.pc = orig->pc;
    REGS.pstate = orig->pstate;

    uint32_t esr = read_sysreg(esr_el1);
    if ((esr & ESR_ELx_BRK64_ISS_COMMENT_MASK) == GDB_COMPILED_DBG_BRK_IMM) {
        g_compiled_brk = true;
    }
}

STUB_TEXT void OsGdbArchFinish(void *stk)
{
    struct PrtRegs *orig = (struct PrtRegs *)stk;

    for (int i = 0; i < sizeof(orig->regs) / sizeof(orig->regs[0]); i++) {
        orig->regs[i] = REGS.regs[i];
    }
    orig->sp = REGS.sp;
    orig->pc = REGS.pc;
    orig->pstate = REGS.pstate;
}

static inline int GdbReadRegUnavailable(U32 regno, U8 *buf)
{
    int size = BYTE_TO_STR_LEN * g_dbgRegDef[regno].size;
    int i;

    for (i = 0; i < size; i++) {
        buf[i] = 'x';
    }
    return size;
}

STUB_TEXT static int GdbReadOneReg(U32 regno, U8 *buf)
{
    int len;

    if (g_dbgRegDef[regno].offset == -1) {
        return GdbReadRegUnavailable(regno, buf);
    }

    len = OsGdbBin2Hex(&RBUF[g_dbgRegDef[regno].offset], g_dbgRegDef[regno].size,
                       buf, g_dbgRegDef[regno].size * BYTE_TO_STR_LEN + 1);

    return len; 
}

STUB_TEXT static int GdbWriteOneReg(U32 regno, U8 *buf)
{
    int len;

    if (g_dbgRegDef[regno].offset == -1) {
        return BYTE_TO_STR_LEN * g_dbgRegDef[regno].size;
    }

    len = OsGdbHex2Bin(buf, g_dbgRegDef[regno].size * BYTE_TO_STR_LEN,
                       &RBUF[g_dbgRegDef[regno].offset], g_dbgRegDef[regno].size);
    return BYTE_TO_STR_LEN * len;
}

STUB_TEXT int OsGdbArchReadAllRegs(U8 *buf, int buflen)
{
    int ret = 0;

    if (buflen < BYTE_TO_STR_LEN * NUMREGBYTES) {
        return 0;
    }

    for (int i = 0; i < DBG_MAX_REG_NUM; i++) {
        ret += GdbReadOneReg(i, &buf[ret]);
    }

    return ret;
}

STUB_TEXT int OsGdbArchWriteAllRegs(U8 *hex, int hexlen)
{
    int ret = 0;

    if (hexlen < BYTE_TO_STR_LEN * NUMREGBYTES) {
        return 0;
    }

    for (int i = 0; i < DBG_MAX_REG_NUM; i++) {
        ret += GdbWriteOneReg(i, &hex[ret]); 
    }

    return ret;
}

STUB_TEXT int OsGdbArchReadReg(U32 regno, U8 *buf, int buflen)
{
    if (regno >= DBG_MAX_REG_NUM || buflen < 2 * g_dbgRegDef[regno].size + 1) {
        return 0;
    }

    return GdbReadOneReg(regno, buf);
}

STUB_TEXT int OsGdbArchWriteReg(U32 regno, U8 *buf, int buflen)
{
    if (regno >= DBG_MAX_REG_NUM || buflen < 2 * g_dbgRegDef[regno].size) {
        return -1;
    }

    return GdbWriteOneReg(regno, buf);
}

STUB_TEXT int OsGdbArchSetSwBkpt(struct GdbBkpt *bkpt)
{
    *(uint32_t *)bkpt->instr = *(uint32_t *)bkpt->addr;
    *(uint32_t *)bkpt->addr = (uint32_t)AARCH64_BREAK_KGDB_DYN_DBG;

    bkpt->type = BP_BREAKPOINT;

    return 0;
}

STUB_TEXT int OsGdbArchRemoveSwBkpt(struct GdbBkpt *bkpt)
{
    *(uint32_t *)bkpt->addr = *(uint32_t *)bkpt->instr;

    return 0;
}

STUB_TEXT void OsGdbArchInit(void)
{
    write_sysreg(0, osdlr_el1);
    write_sysreg(0, oslar_el1);

    __asm__ volatile(
        "dsb st\n"
        "brk %0\n"
        "dsb st\n"
        : : "I" (GDB_COMPILED_DBG_BRK_IMM));
}

STUB_TEXT static void OsCacheFlush(unsigned long addr_start, unsigned long addr_end)
{
    uint64_t cache_line_size;

    __asm__ volatile(
        "mrs %0, ctr_el0\n"
        : "=r"(cache_line_size)
        : : "memory");

    cache_line_size = 0x4UL << ((cache_line_size >> 16) & 0xF);

    addr_start &= ~(cache_line_size - 1);

    for (; addr_start < addr_end;) {
        __asm__ volatile(
            "dc cvac, %0\n"
            "ic ivau, %0\n"
            :
            : "r"(addr_start)
            : "memory");
        addr_start += cache_line_size;
    }

    __asm__ volatile(
        "dsb  sy\n"
        "isb  sy\n"
        : : : "memory");
}

STUB_TEXT void GdbFlushSwBkptAddr(unsigned long addr)
{
    /* Force flush instruction cache if it was outside the mm */
    OsCacheFlush(addr, addr + BREAK_INSTR_SIZE);
}
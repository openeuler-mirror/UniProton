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
#include "prt_notifier.h"
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

STUB_DATA static bool g_stopByCtrlc;

STUB_DATA static bool g_needStepFlg;

STUB_DATA static uint8_t g_alignedLenArray[] = {1,2,4,4,8,8,8,8};

STUB_DATA static int g_maxWrpCnt;

STUB_DATA static int g_maxBrpCnt;

STUB_DATA static Watchpoint_t g_wrpArray[ARM_MAX_WRP];

STUB_DATA static HwBreakpoint_t g_brpArray[ARM_MAX_BRP];

STUB_DATA static uint8_t g_basArray[] = {
    ARM_BREAKPOINT_LEN_1, ARM_BREAKPOINT_LEN_2, ARM_BREAKPOINT_LEN_3,
    ARM_BREAKPOINT_LEN_4, ARM_BREAKPOINT_LEN_5, ARM_BREAKPOINT_LEN_6,
    ARM_BREAKPOINT_LEN_7, ARM_BREAKPOINT_LEN_8,
};

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
}

STUB_TEXT void OsGdbArchStep(void)
{
    if (g_compiled_brk) {
        REGS.pc += BREAK_INSTR_SIZE;
        g_compiled_brk = false;
    }

    REGS.pstate |= DBG_SPSR_SS;
    REGS.pstate &= ~DBG_SPSR_D;
    mdscr_write(mdscr_read() | DBG_MDSCR_SS | DBG_MDSCR_KDE);
}

STUB_TEXT void OsGdbMarkStep(uint64_t *sp)
{
    if (LIKELY(!g_needStepFlg)) {
        return;
    }
    g_needStepFlg = false;
    sp[1] |= DBG_SPSR_SS;
    sp[1] &= ~DBG_SPSR_D;
    mdscr_write(mdscr_read() | DBG_MDSCR_SS | DBG_MDSCR_KDE);
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
    g_debugContext.far = 0;

    uint32_t esr = read_sysreg(esr_el1);
    g_debugContext.ec = ESR_ELx_EC(esr);
    if ((esr & ESR_ELx_BRK64_ISS_COMMENT_MASK) == GDB_COMPILED_DBG_BRK_IMM) {
        g_compiled_brk = true;
    } else if (ESR_ELx_EC(esr) == ESR_ELx_EC_WATCHPT_CUR) {
        g_debugContext.far = read_sysreg(far_el1);
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

static inline void InsertCompiledBrk()
{
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

static STUB_TEXT uint64_t ReadWbReg(int reg, int n)
{
    uint64_t val = 0;

    switch (reg + n) {
    GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BVR, AARCH64_DBG_REG_NAME_BVR, val);
    GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_BCR, AARCH64_DBG_REG_NAME_BCR, val);
    GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WVR, AARCH64_DBG_REG_NAME_WVR, val);
    GEN_READ_WB_REG_CASES(AARCH64_DBG_REG_WCR, AARCH64_DBG_REG_NAME_WCR, val);
    default:
        break;
    }

    return val;
}

static STUB_TEXT void WriteWbReg(int reg, int n, uint64_t val)
{
    switch (reg + n) {
    GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_BVR, AARCH64_DBG_REG_NAME_BVR, val);
    GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_BCR, AARCH64_DBG_REG_NAME_BCR, val);
    GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_WVR, AARCH64_DBG_REG_NAME_WVR, val);
    GEN_WRITE_WB_REG_CASES(AARCH64_DBG_REG_WCR, AARCH64_DBG_REG_NAME_WCR, val);
    default:
        break;
    }
    __asm__ volatile("isb" : : : "memory");
}

#define FIND_FREE_SLOT(array, size) ({ \
    int i = 0, idx = -1;               \
    for (i; i < size; i++) {           \
        if (array[i].state == 0) {     \
            idx = i;                   \
            break;                     \
        }                              \
    }                                  \
    idx;                               \
});

STUB_TEXT void OsGdbArchInit(void)
{
    /* Determine number of BRP/WRP registers available. */
    uint64_t aa64dfr0 = read_sysreg(id_aa64dfr0_el1);

    g_maxBrpCnt = AA64DFR0_BRPS_VAL(aa64dfr0);
    g_maxWrpCnt = AA64DFR0_WRPS_VAL(aa64dfr0);

    for (int i = 0; i < g_maxBrpCnt; i++) {
        WriteWbReg(AARCH64_DBG_REG_BVR, i, 0);
        WriteWbReg(AARCH64_DBG_REG_BCR, i, 0);
    }
    for (int i = 0; i < g_maxWrpCnt; i++) {
        WriteWbReg(AARCH64_DBG_REG_WVR, i, 0);
        WriteWbReg(AARCH64_DBG_REG_WCR, i, 0);
    }
    write_sysreg(0, osdlr_el1);
    write_sysreg(0, oslar_el1);

    InsertCompiledBrk();
}

static inline uint32_t EncodeCtrl(struct HwBreakpointCtrl ctrl)
{
    return (ctrl.len << 5) | (ctrl.type << 3) | (ctrl.privilege << 1) | ctrl.enabled;
}

static inline void DecodeCtrl(uint32_t reg, struct HwBreakpointCtrl *ctrl)
{
    ctrl->enabled = reg & 0x1;
    reg >>= 1;
    ctrl->privilege = reg & 0x3;
    reg >>= 2;
    ctrl->type = reg & 0x3;
    reg >>= 2;
    ctrl->len = reg & 0xff;
}

static STUB_TEXT int AddBrp(uintptr_t addr, int len)
{
    struct HwBreakpointCtrl ctrl = {0};
    int idx = FIND_FREE_SLOT(g_brpArray, g_maxBrpCnt);
    if (idx == -1) {
        return -1;
    }
    ctrl.enabled = 1;
    ctrl.len = ARM_BREAKPOINT_LEN_4;
    ctrl.privilege = AARCH64_BREAKPOINT_EL1;
    g_brpArray[idx].addr = addr;
    g_brpArray[idx].state = 1;
    g_brpArray[idx].ctrl = EncodeCtrl(ctrl);
    g_brpArray[idx].len = 4;
    return 0;
}

static STUB_TEXT int RemoveBrp(uintptr_t addr, int len)
{
    int idx = -1;
    for (int i = 0; i < g_maxBrpCnt; i++) {
        if (g_brpArray[i].state == 0) {
            continue;
        }
        if (g_brpArray[i].addr == addr) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        return -1;
    }
    g_brpArray[idx].state = 0;
    return 0;
}

static STUB_TEXT int AlignWatchpoint(uintptr_t addr, uint32_t len, enum GdbBkptType bptype, Watchpoint_t *wp)
{
    uint32_t offset, alignedLen;
    uintptr_t alignedAddr;
    struct HwBreakpointCtrl ctrl = {0};

    if (len == 0 || wp == NULL) {
        return -1;
    }
    offset = addr & 7;
    alignedAddr = addr - offset;
    if (offset + len > 8) {
        return -1;
    }

    alignedLen = g_alignedLenArray[offset + len - 1];
    wp->origAddr = addr;
    wp->origLen = len;
    wp->addr = alignedAddr;
    wp->len = alignedLen;
    wp->state = 1;
    ctrl.enabled = 1;
    ctrl.len = g_basArray[offset + len];
    ctrl.privilege = AARCH64_BREAKPOINT_EL1;
    if (bptype == BP_WRITE_WATCHPOINT) {
        ctrl.type = ARM_BREAKPOINT_STORE;
    } else if (bptype == BP_READ_WATCHPOINT) {
        ctrl.type = ARM_BREAKPOINT_LOAD;
    } else {
        ctrl.type = ARM_BREAKPOINT_STORE | ARM_BREAKPOINT_LOAD;
    }
    wp->ctrl = EncodeCtrl(ctrl);
    return 0;
}

static STUB_TEXT int AddWrp(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    struct Watchpoint *wp = NULL;
    int idx = FIND_FREE_SLOT(g_wrpArray, g_maxWrpCnt);
    if (idx == -1) {
        return -1;
    }
    wp = &g_wrpArray[idx];
    if (AlignWatchpoint(addr, len, bptype, wp)) {
        return -1;
    }
    return 0;
}

static STUB_TEXT int RemoveWrp(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    struct Watchpoint wp = {0};
    int idx = -1;
    AlignWatchpoint(addr, len, bptype, &wp);
    for (int i = 0; i < g_maxWrpCnt; i++) {
        if (g_wrpArray[i].state == 0) {
            continue;
        }
        if (g_wrpArray[i].addr == wp.addr && g_wrpArray[i].len == wp.len) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        return -1;
    }
    g_wrpArray[idx].state = 0;
    return 0;
}

STUB_TEXT int OsGdbArchSetHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    if (bptype == BP_HARDWARE_BREAKPOINT) {
        return AddBrp(addr, len);
    } else if (bptype >= BP_WRITE_WATCHPOINT && bptype <= BP_ACCESS_WATCHPOINT) {
        return AddWrp(addr, len, bptype);
    } else {
        return -1;
    }
}

STUB_TEXT int OsGdbArchRemoveHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    if (bptype == BP_HARDWARE_BREAKPOINT) {
        return RemoveBrp(addr, len);
    } else if (bptype >= BP_WRITE_WATCHPOINT && bptype <= BP_ACCESS_WATCHPOINT) {
        return RemoveWrp(addr, len, bptype);
    } else {
        return -1;
    }
}

STUB_TEXT void OsGdbArchRemoveAllHwBkpts(void)
{
    for (int i = 0; i < g_maxBrpCnt; i++) {
        g_brpArray[i].state = 0;
    }
    for (int i = 0; i < g_maxWrpCnt; i++) {
        g_wrpArray[i].state = 0;
    }
    OsGdbArchDisableHwBkpts();
}

STUB_TEXT void OsGdbArchCorrectHwBkpts(void)
{
    for (int i = 0; i < g_maxBrpCnt; i++) {
        if (g_brpArray[i].state == 0) {
            continue;
        }
        WriteWbReg(AARCH64_DBG_REG_BVR, i, g_brpArray[i].addr);
        WriteWbReg(AARCH64_DBG_REG_BCR, i, g_brpArray[i].ctrl);
    }
    for (int i = 0; i < g_maxWrpCnt; i++) {
        if (g_wrpArray[i].state == 0) {
            continue;
        }
        WriteWbReg(AARCH64_DBG_REG_WVR, i, g_wrpArray[i].addr);
        WriteWbReg(AARCH64_DBG_REG_WCR, i, g_wrpArray[i].ctrl);
    }
    mdscr_write(mdscr_read() | DBG_MDSCR_KDE | DBG_MDSCR_MDE);
}

STUB_TEXT int OsGdbArchHitHwBkpt(uintptr_t *addr, unsigned *type)
{
    uint64_t far;
    struct HwBreakpointCtrl ctrl = {0};
    if (addr == NULL || type == NULL || g_debugContext.far == 0) {
        return 0;
    }
    if (g_debugContext.ec != ESR_ELx_EC_WATCHPT_CUR) {
        return 0;
    }
    far = g_debugContext.far;
    for (int i = 0; i < g_maxWrpCnt; i++) {
        if (g_wrpArray[i].state == 0) {
            continue;
        }
        if (g_wrpArray[i].addr <= far && g_wrpArray[i].addr + g_wrpArray[i].len > far) {
            *addr = g_wrpArray[i].origAddr;
            DecodeCtrl(g_wrpArray[i].ctrl, &ctrl);
            if (ctrl.type == ARM_BREAKPOINT_STORE) {
                *type = BP_WRITE_WATCHPOINT;
            } else if (ctrl.type == ARM_BREAKPOINT_LOAD) {
                *type = BP_READ_WATCHPOINT;
            } else if (ctrl.type == ARM_BREAKPOINT_LOAD | ARM_BREAKPOINT_STORE) {
                *type = BP_ACCESS_WATCHPOINT;
            }
            return 1;
        }
    }
    return 0;
}

STUB_TEXT void OsGdbArchDisableHwBkpts()
{
    for (int i = 0; i < g_maxBrpCnt; i++) {
        WriteWbReg(AARCH64_DBG_REG_BCR, i, 0);
    }
    for (int i = 0; i < g_maxWrpCnt; i++) {
        WriteWbReg(AARCH64_DBG_REG_WCR, i, 0);
    }
    mdscr_write(mdscr_read() & DBG_MDSCR_MASK);
}

STUB_TEXT int OsGdbArchNotifyDie(int action, void *data)
{
    g_stopByCtrlc = true;
#ifdef LAZY_STOP
    g_needStepFlg = true;
#else
    InsertCompiledBrk();
#endif
    return NOTIFY_STOP;
}

STUB_TEXT int OsGdbGetStopReason()
{
    if (g_stopByCtrlc) {
        g_stopByCtrlc = false;
        return 2;
    }
    return 5;
}
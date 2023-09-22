#include "prt_typedef.h"
#include "gdbstub.h"
#include "gdbstub_common.h"
#include "rsp_utils.h"

STUB_DATA struct GdbCtx g_debugContext;

static STUB_DATA struct DbgRegDef g_dbgRegDef[DBG_MAX_REG_NUM] =
{
    { "ax", 8, offsetof(struct PrtRegs, ax) },
    { "bx", 8, offsetof(struct PrtRegs, bx) },
    { "cx", 8, offsetof(struct PrtRegs, cx) },
    { "dx", 8, offsetof(struct PrtRegs, dx) },
    { "si", 8, offsetof(struct PrtRegs, si) },
    { "di", 8, offsetof(struct PrtRegs, di) },
    { "bp", 8, offsetof(struct PrtRegs, bp) },
    { "sp", 8, offsetof(struct PrtRegs, sp) },
    { "r8", 8, offsetof(struct PrtRegs, r8) },
    { "r9", 8, offsetof(struct PrtRegs, r9) },
    { "r10", 8, offsetof(struct PrtRegs, r10) },
    { "r11", 8, offsetof(struct PrtRegs, r11) },
    { "r12", 8, offsetof(struct PrtRegs, r12) },
    { "r13", 8, offsetof(struct PrtRegs, r13) },
    { "r14", 8, offsetof(struct PrtRegs, r14) },
    { "r15", 8, offsetof(struct PrtRegs, r15) },
    { "ip", 8, offsetof(struct PrtRegs, ip) },
    { "flags", 4, offsetof(struct PrtRegs, flags) },
    { "cs", 4, offsetof(struct PrtRegs, cs) },
    { "ss", 4, offsetof(struct PrtRegs, ss) },
    { "ds", 4, -1 },
    { "es", 4, -1 },
    { "fs", 4, -1 },
    { "gs", 4, -1 },
};

#define REGS (g_debugContext.regs.regs)
#define RBUF (g_debugContext.regs.rbuf)

STUB_TEXT void OsGdbArchContinue(void)
{
    /* Clear the TRAP FLAG bit */
    REGS.flags &= ~BIT(8);
}

STUB_TEXT void OsGdbArchStep(void)
{
    /* Set the TRAP FLAG bit */
    REGS.flags |= BIT(8);
}

STUB_TEXT void OsGdbArchPrepare(void *stk)
{
    struct PrtRegs *orig = (struct PrtRegs *)stk;
    REGS.ax = orig->ax;
    REGS.bx = orig->bx;
    REGS.cx = orig->cx;
    REGS.dx = orig->dx;
    REGS.si = orig->si;
    REGS.di = orig->di;
    REGS.bp = orig->bp;
    REGS.r8 = orig->r8;
    REGS.r9 = orig->r9;
    REGS.r10 = orig->r10;
    REGS.r11 = orig->r11;
    REGS.r12 = orig->r12;
    REGS.r13 = orig->r13;
    REGS.r14 = orig->r14;
    REGS.r15 = orig->r15;
    REGS.unused0 = orig->unused0;
    REGS.unused1 = orig->unused1;
    REGS.unused2 = orig->unused2;
    REGS.unused3 = orig->unused3;
    REGS.ip = orig->ip;
    REGS.cs = orig->cs;
    REGS.flags = orig->flags;
    REGS.sp = orig->sp;
    REGS.ss = orig->ss;
}

STUB_TEXT void OsGdbArchFinish(void *stk)
{
    struct PrtRegs *orig = (struct PrtRegs *)stk;
    orig->ax = REGS.ax;
    orig->bx = REGS.bx;
    orig->cx = REGS.cx;
    orig->dx = REGS.dx;
    orig->si = REGS.si;
    orig->di = REGS.di;
    orig->bp = REGS.bp;
    orig->r8 = REGS.r8;
    orig->r9 = REGS.r9;
    orig->r10 = REGS.r10;
    orig->r11 = REGS.r11;
    orig->r12 = REGS.r12;
    orig->r13 = REGS.r13;
    orig->r14 = REGS.r14;
    orig->r15 = REGS.r15;
    orig->unused0 = REGS.unused0;
    orig->unused1 = REGS.unused1;
    orig->unused2 = REGS.unused2;
    orig->unused3 = REGS.unused3;
    orig->ip = REGS.ip;
    orig->cs = REGS.cs;
    orig->flags = REGS.flags;
    orig->sp = REGS.sp;
    orig->ss = REGS.ss;

}

INLINE int GdbReadRegUnavailable(U32 regno, U8 *buf)
{
    int size = 2 * g_dbgRegDef[regno].size;
    int i;

    for (i = 0; i < size; i++) {
        buf[i] = 'x';
    }
    return size;
}

INLINE int GdbReadOneReg(U32 regno, U8 *buf)
{
    if (g_dbgRegDef[regno].offset == -1) {
        return GdbReadRegUnavailable(regno, buf);
    }
    return OsGdbBin2Hex(&RBUF[g_dbgRegDef[regno].offset],
                    g_dbgRegDef[regno].size,
                    buf,
                    g_dbgRegDef[regno].size * 2);
}

INLINE int GdbWriteOneReg(U32 regno, U8 *buf)
{
    if (g_dbgRegDef[regno].offset == -1) {
        return 2 * g_dbgRegDef[regno].size;
    }
    return 2 * OsGdbHex2Bin(buf,
                    g_dbgRegDef[regno].size * 2,
                    &RBUF[g_dbgRegDef[regno].offset],
                    g_dbgRegDef[regno].size);
}

STUB_TEXT int OsGdbArchReadReg(U32 regno, U8 *buf, int buflen)
{
    if ((regno >= DBG_MAX_REG_NUM) || 
        (buflen < 2 * g_dbgRegDef[regno].size + 1)) {
        return 0;
    }
    return GdbReadOneReg(regno, buf);
}

STUB_TEXT int OsGdbArchReadAllRegs(U8 *buf, int buflen)
{
    int ret = 0;

    if (buflen < 2 * NUMREGBYTES) {
        return 0;
    }
    for (int i = 0; i < DBG_MAX_REG_NUM; i++) {
        ret += GdbReadOneReg(i, &buf[ret]);
    }
    return ret;
}

STUB_TEXT int OsGdbArchWriteReg(U32 regno, U8 *buf, int buflen)
{
    if (regno == GDB_SP || regno == GDB_ORIG_AX) {
        return 0;
    }
    if (regno >= DBG_MAX_REG_NUM || 
        buflen < 2 * g_dbgRegDef[regno].size) {
        return -1;
    }
    return GdbWriteOneReg(regno, buf);
}

STUB_TEXT int OsGdbArchWriteAllRegs(U8 *hex, int hexlen)
{
    int ret = 0;

    if (hexlen < 2 * NUMREGBYTES) {
        return 0;
    }

    for (int i = 0; i < DBG_MAX_REG_NUM; i++) {
        ret += GdbWriteOneReg(i, &hex[ret]); 
    }
    return ret;
}

STUB_TEXT int OsGdbArchSetSwBkpt(struct GdbBkpt *bkpt)
{
    bkpt->type = BP_BREAKPOINT;

    bkpt->instr[0] = *((char *)bkpt->addr);
    *((char *)bkpt->addr) = BREAK_INST;

    return 0;
}

STUB_TEXT int OsGdbArchRemoveSwBkpt(struct GdbBkpt *bkpt)
{
    *((char *)bkpt->addr) = bkpt->instr[0];
    return 0;
}

STUB_TEXT void OsGdbArchInit(void)
{
    __asm__ volatile("int3");
}

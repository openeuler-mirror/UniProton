#include "prt_typedef.h"
#include "prt_notifier.h"
#include "gdbstub.h"
#include "gdbstub_common.h"
#include "rsp_utils.h"

/* Available HW breakpoint length encodings */
#define X86_BREAKPOINT_LEN_X		0x40
#define X86_BREAKPOINT_LEN_1		0x40
#define X86_BREAKPOINT_LEN_2		0x44
#define X86_BREAKPOINT_LEN_4		0x4c
#define X86_BREAKPOINT_LEN_8		0x48

/* Available HW breakpoint type encodings */

/* trigger on instruction execute */
#define X86_BREAKPOINT_EXECUTE	0x80
/* trigger on memory write */
#define X86_BREAKPOINT_WRITE	0x81
/* trigger on memory read or write */
#define X86_BREAKPOINT_RW	0x83

/* Total number of available HW breakpoint registers */
#define HBP_NUM 4

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

static STUB_DATA struct HwBrkInfo g_brpArray[HBP_NUM];
static STUB_DATA unsigned long g_oldDr7;
static STUB_DATA int g_stopReason;

#define WRITE_DBG_REG_CASE(no, val)             \
    case no:                                    \
        asm("mov %0, %%db" #no ::"r" (val));    \
        break

#define READ_DBG_REG_CASE(no, val)              \
    case no:                                    \
        asm("mov %%db" #no ", %0" :"=r" (val)); \
        break

INLINE unsigned long GetDbgReg(int regno)
{
    unsigned long val = 0;

    switch (regno) {
    READ_DBG_REG_CASE(0, val);
    READ_DBG_REG_CASE(1, val);
    READ_DBG_REG_CASE(2, val);
    READ_DBG_REG_CASE(3, val);
    READ_DBG_REG_CASE(6, val);
    case DR7:
        /*
         * Apply __FORCE_ORDER to DR7 reads to forbid re-ordering them
         * with other code.
         *
         * This is needed because a DR7 access can cause a #VC exception
         * when running under SEV-ES. Taking a #VC exception is not a
         * safe thing to do just anywhere in the entry code and
         * re-ordering might place the access into an unsafe location.
         *
         * This happened in the NMI handler, where the DR7 read was
         * re-ordered to happen before the call to sev_es_ist_enter(),
         * causing stack recursion.
         */
        asm volatile("mov %%db7, %0" : "=r" (val) : __FORCE_ORDER);
        break;
    default:
        break;
    }
    return val;
}

INLINE void SetDbgReg(int regno, unsigned long value)
{
    switch (regno) {
    WRITE_DBG_REG_CASE(0, value);
    WRITE_DBG_REG_CASE(1, value);
    WRITE_DBG_REG_CASE(2, value);
    WRITE_DBG_REG_CASE(3, value);
    WRITE_DBG_REG_CASE(6, value);
    case DR7:
        /*
         * Apply __FORCE_ORDER to DR7 writes to forbid re-ordering them
         * with other code.
         *
         * While is didn't happen with a DR7 write (see the DR7 read
         * comment above which explains where it happened), add the
         * __FORCE_ORDER here too to avoid similar problems in the
         * future.
         */
        asm volatile("mov %0, %%db7"    ::"r" (value), __FORCE_ORDER);
        break;
    default:
        break;
    }
}

INLINE void ResetDbgReg(void)
{
    /* Zero the control register for HW Breakpoint */
    SetDbgReg(DR7, 0UL);

    /* Zero-out the individual HW breakpoint address registers */
    SetDbgReg(DR0, 0UL);
    SetDbgReg(DR1, 0UL);
    SetDbgReg(DR2, 0UL);
    SetDbgReg(DR3, 0UL);
}

/*
 * Encode the length, type, Exact, and Enable bits for a particular breakpoint
 * as stored in debug register 7.
 */
INLINE unsigned long EncodeDr7(int drnum, unsigned int len, unsigned int type)
{
    unsigned long bp_info;

    bp_info = (len | type) & 0xf;
    bp_info <<= (DR_CONTROL_SHIFT + drnum * DR_CONTROL_SIZE);
    bp_info |= (DR_GLOBAL_ENABLE << (drnum * DR_ENABLE_SIZE));

    return bp_info | DR_GLOBAL_SLOWDOWN;
}

INLINE bool IsIntersect(unsigned long addr1, unsigned long size1,
                   unsigned long addr2, unsigned long size2)
{
    return addr1 + size1 > addr2 && addr2 + size2 > addr1;
}

STUB_TEXT int OsGdbArchHitHwBkpt(uintptr_t *addr, unsigned *type)
{
    int i;
	unsigned wtype;

    unsigned long dr6 = GetDbgReg(DR6);
    /* Do an early return if no trap bits are set in DR6 */
    if ((dr6 & DR_TRAP_BITS) == 0) {
        return 0;
    }

    /* Handle all the breakpoints that were triggered */
    for (i = 0; i < HBP_NUM; ++i) {
        if (!(dr6 & (DR_TRAP0 << i))) {
            continue;
        }
        *addr = GetDbgReg(i);
		wtype = g_brpArray[i].type;
		if (wtype == X86_BREAKPOINT_RW) {
			*type = BP_ACCESS_WATCHPOINT;
		} else {
			*type = BP_WRITE_WATCHPOINT;
		}
        return 1;
    }
    return 0;
}

STUB_TEXT void OsGdbArchCorrectHwBkpts(void)
{
    int i;
    unsigned long dr7 = 0;
    for (i = 0; i < HBP_NUM; i++) {
        if (!g_brpArray[i].enabled) {
            continue;
        }
        SetDbgReg(i, g_brpArray[i].addr);
        dr7 |= EncodeDr7(i, g_brpArray[i].len, g_brpArray[i].type);
    }

    SetDbgReg(DR6, DR6_RESERVED);
    SetDbgReg(DR7, dr7);

}

STUB_TEXT void OsGdbArchRemoveAllHwBkpts(void)
{
    int i;

    for (i = 0; i < HBP_NUM; i++) {
        if (!g_brpArray[i].enabled) {
            continue;
        }
        g_brpArray[i].enabled = 0;
    }
    ResetDbgReg();
}

STUB_TEXT int OsGdbArchRemoveHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    int i;

    for (i = 0; i < HBP_NUM; i++) {
        if (g_brpArray[i].addr == addr && g_brpArray[i].enabled) {
            break;
        }
    }
    if (i == HBP_NUM) {
        return -1;
    }
    g_brpArray[i].enabled = 0;
    return 0;
}

STUB_TEXT int OsGdbArchSetHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype)
{
    int i;

    for (i = 0; i < HBP_NUM; i++) {
        if (g_brpArray[i].enabled &&
            IsIntersect(addr, len, g_brpArray[i].addr, g_brpArray[i].len)){
            return -1;
        }
    }
    for (i = 0; i < HBP_NUM; i++) {
        if (!g_brpArray[i].enabled) {
            break;
        }
    }
    if (i == HBP_NUM) {
        return -1;
    }

    switch (bptype) {
    case BP_WRITE_WATCHPOINT:
        g_brpArray[i].type = X86_BREAKPOINT_WRITE;
        break;
    case BP_ACCESS_WATCHPOINT:
        g_brpArray[i].type = X86_BREAKPOINT_RW;
        break;
    default:
        return -1;
    }

    switch (len) {
    case 1:
        g_brpArray[i].len = X86_BREAKPOINT_LEN_1;
        break;
    case 2:
        g_brpArray[i].len = X86_BREAKPOINT_LEN_2;
        break;
    case 4:
        g_brpArray[i].len = X86_BREAKPOINT_LEN_4;
        break;
    case 8:
        g_brpArray[i].len = X86_BREAKPOINT_LEN_8;
        break;
    default:
        return -1;
    }
    g_brpArray[i].addr = addr;
    g_brpArray[i].enabled = 1;
    return 0;
}

/**
 *    Disable hardware debugging while we in gdb stub.
 *
 *    This function will be called if the particular architecture must
 *    disable hardware debugging while it is processing gdb packets or
 *    handling exception.
 */
STUB_TEXT void OsGdbArchDisableHwBkpts()
{
    g_oldDr7 = GetDbgReg(DR7);
    SetDbgReg(DR7, 0UL);
}

STUB_TEXT int OsGdbArchNotifyDie(int action, void *data)
{
    if (action == 1 || action == 3) {// DEBUG & BREAKPOINT
        g_stopReason = 5; // SIGTRAP
    } else { // NMI
        g_stopReason = 2; // SIGINT
    }
    OsGdbHandleException(data);
    return NOTIFY_STOP;
}

STUB_TEXT int OsGdbGetStopReason()
{
    return g_stopReason;
}
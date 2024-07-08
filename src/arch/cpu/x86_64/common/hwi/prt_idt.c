#include "prt_vector.h"
#include "prt_lapic.h"
#include "prt_hwi_external.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"
#include "prt_idt.h"
#include "prt_buildef.h"
#ifdef OS_GDB_STUB
#include "prt_gdbstub_ext.h"
#endif
extern void OsMainSchedule(void);

struct IdtEntry g_idtr[VECTOR_MAX_COUNT];

static void OsIdtrInit(unsigned int index, U64 base, U8 type)
{
    struct IdtEntry *entry = &g_idtr[index];

    entry->lobase = base & 0xffff;
    entry->hibase = (base >> 16) & 0xffff;
    entry->xhibase = (base >> 32) & 0xffffffff;
    entry->selector = KERNEL_CS;
    entry->ist = 0;
    entry->reserved = 0;
    entry->type = type;
    entry->zero = 0;
    entry->dpl = 0;
    entry->present = 1;
    entry->reserved1 = 0;
    return;
}

static inline void OsloadIdt(void* p, unsigned long size)
{
    volatile unsigned short pd[5];
    pd[0] = size - 1;
    pd[1] = (U64)p;
    pd[2] = (U64)p >> 16;
    pd[3] = (U64)p >> 32;
    pd[4] = (U64)p >> 48;
    __asm__ volatile("lidt (%0)" : : "r" (pd));
    return;
}

void OsIdtIrqEnable(U32 index)
{
    (void)index;
    return;
}

void OsIdtIrqDisable(U32 index)
{
    (void)index;
    return;
}

void OsIdtInit(void)
{
    U32 loop;
    memset_s((void *)g_idtr, sizeof(struct IdtEntry) * VECTOR_MAX_COUNT, 0, sizeof(struct IdtEntry) * VECTOR_MAX_COUNT);

    for (loop = 0; loop < VECTOR_MAX_COUNT; loop++) {
        OsIdtrInit(loop, g_osVectors[loop], INTR_GATE);
    }

    OsIdtrInit(HWI_DE, (U64)g_osVectors[HWI_DE], TRAP_GATE);
    OsIdtrInit(HWI_BP, (U64)g_osVectors[HWI_BP], TRAP_GATE);
    OsIdtrInit(HWI_UD, (U64)g_osVectors[6], TRAP_GATE);
    OsIdtrInit(HWI_NM, (U64)g_osVectors[7], TRAP_GATE);
    OsIdtrInit(HWI_GP, (U64)g_osVectors[13], TRAP_GATE);
    OsIdtrInit(HWI_PF, (U64)g_osVectors[14], TRAP_GATE);
    OsIdtrInit(HWI_MF, (U64)g_osVectors[16], TRAP_GATE);
    OsIdtrInit(HWI_XM, (U64)g_osVectors[19], TRAP_GATE);

    OsloadIdt(g_idtr, sizeof(struct IdtEntry) * VECTOR_MAX_COUNT);
    return;
}

void OsHwiTail(void)
{
    U64 irqStartTime = 0;
    if (TICK_NO_RESPOND_CNT > 0) {
        if (UNI_FLAG & OS_FLG_TICK_ACTIVE) {
            // OsTskContextLoad，回到被打断的tick处理现场
            return;
        }
#if defined(OS_OPTION_RR_SCHED) && defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
        irqStartTime = OsCurCycleGet64();
#endif
        UNI_FLAG |= OS_FLG_TICK_ACTIVE;

        do {
            // tickISR，这里开中断
            g_tickDispatcher();
            TICK_NO_RESPOND_CNT--;
        } while (TICK_NO_RESPOND_CNT > 0);

        UNI_FLAG &= ~OS_FLG_TICK_ACTIVE;
        OS_IRQ_TIME_RECORD(irqStartTime);
    }
#if defined(OS_OPTION_RR_SCHED)
    OsHwiEndCheckTimeSlice(OsCurCycleGet64());
#endif

    OsMainSchedule();
    return;
}

U64 ReadCr2(void)
{
    U64 rax;
    __asm__ volatile("mov %%cr2, %0" : "=a"(rax));
    return rax;
}

#ifdef OS_GDB_STUB
#include "prt_notifier.h"
STUB_TEXT void OsHwiDbgExcProc(U64 stackFrame)
{
    struct StackFrame *frame = (struct StackFrame *)stackFrame;
    OsNotifyDie(frame->intNumber, frame);
}
#endif

void OsHwiDispatchProc(U64 stackFrame)
{
    struct StackFrame *frame;
    U64 irqStartTime = 0;
    frame = (struct StackFrame *)(stackFrame + 0x200);
#if defined(OS_OPTION_RR_SCHED) && defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
    if ((UNI_FLAG & OS_FLG_HWI_ACTIVE) == 0) {
        irqStartTime = OsCurCycleGet64();
    }
#endif
    UNI_FLAG |= OS_FLG_HWI_ACTIVE;
    OS_INT_COUNT++;

    if (frame->intNumber >= 0x20 && frame->intNumber < 0x100) {
        OsHwiHookDispatcher(frame->intNumber);
    } else if (frame->intNumber == 0x2) {
        OsHwiHookDispatcher(frame->intNumber);
    } else {
        printf("rax    0x%16llx  rbx 0x%16llx\r\n", frame->rax, frame->rbx);
        printf("rcx    0x%16llx  rdx 0x%16llx\r\n", frame->rcx, frame->rdx);
        printf("rsi    0x%16llx  rdi 0x%16llx\r\n", frame->rsi, frame->rdi);
        printf("rbp    0x%16llx  r8  0x%16llx\r\n", frame->rbp, frame->r8);
        printf("r9     0x%16llx  r10 0x%16llx\r\n", frame->r9, frame->r10);
        printf("r11    0x%16llx  r12 0x%16llx\r\n", frame->r11, frame->r12);
        printf("r13    0x%16llx  r14 0x%16llx\r\n", frame->r13, frame->r14);
        printf("r15    0x%16llx  rip 0x%16llx\r\n", frame->r15, frame->rip);
        printf("rflags 0x%16llx  cs  0x%16llx\r\n", frame->rFlags, frame->cs);
        printf("ss     0x%16llx  rsp 0x%16llx\r\n", frame->ss, frame->rsp);
        printf("irq    0x%16llx  err 0x%16llx\r\n", frame->intNumber, frame->error);
        printf("CR2    0x%16llx\r\n", ReadCr2());

        while (1);
    }

    // 清除中断位
    OsWriteMsr(X2APIC_EOI, 0);

    OS_INT_COUNT--;
    if (OS_INT_COUNT > 0) {
        return;
    }
    OS_IRQ_TIME_RECORD(irqStartTime);
    UNI_FLAG &= ~OS_FLG_HWI_ACTIVE;

    OsHwiTail();
    return;
}

#include <nuttx/irq.h>
#include "prt_irq_internal.h"

#if defined(OS_ARCH_ARMV7_M)
#define OS_NXAL_IRQ_2_PTR(irq) ((irq) - OS_MX_SYS_VECTOR_CNT)
#else
#define OS_NXAL_IRQ_2_PTR(irq) (irq)
#endif
#define HWI_DEFAULT_PRIOR    1

int irq_attach(int irq, xcpt_t isr, FAR void *arg)
{
    U32 ret;
    HwiHandle hwiNum = OS_NXAL_IRQ_2_PTR(irq);
    uintptr_t intSave = OsIntLock();
    if (isr == NULL) {
        ret = PRT_HwiDelete(hwiNum);
    } else {
        ret = PRT_HwiSetAttr(hwiNum, HWI_DEFAULT_PRIOR, OS_HWI_MODE_ENGROSS);
        if (ret != OS_OK) {
            OsIntRestore(intSave);
            return (int)ret;
        }
        ret = PRT_HwiCreate(hwiNum, (HwiProcFunc)isr, (HwiArg)arg);
    }

    OsIntRestore(intSave);
    return (int)ret;
}

void up_enable_irq(int irq)
{
    (void)PRT_HwiEnable(OS_NXAL_IRQ_2_PTR((U32)irq));
}

void up_disable_irq(int irq)
{
    (void)PRT_HwiDisable(OS_NXAL_IRQ_2_PTR((U32)irq));
}
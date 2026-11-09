/* SPDX-License-Identifier: MulanPSL-2.0 */
#include <mica/platform/system/uniproton/irq.h>

#ifdef OS_OPTION_SMP
#error "libmica domain IRQ bridge is UP-only"
#endif

/* Domain-init serializes the single physical IRQ registration. Request does
 * not enable; native HwiArg carries the argument without a second record. */
static mica_irq_handler_t bridge_fn;

static void dispatch(HwiArg arg)
{
    bridge_fn((void *)arg);
}

int mica_request_irq(unsigned irq, mica_irq_handler_t fn, void *arg)
{
    if (!fn)
        return -EINVAL;
    if (bridge_fn)
        return -EBUSY;

    bridge_fn = fn;
    U32 ret = PRT_HwiCreate(irq, dispatch, (HwiArg)arg);

    if (ret != OS_OK)
        bridge_fn = NULL;
    return ret == OS_OK ? 0 : -EIO;
}

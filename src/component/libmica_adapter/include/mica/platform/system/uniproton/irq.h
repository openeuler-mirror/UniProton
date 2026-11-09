/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_IRQ_H
#define MICA_UNIPROTON_IRQ_H
#include <mica/platform/time.h>
#include <errno.h>
#include "prt_hwi.h"
typedef void (*mica_irq_handler_t)(void *);
int mica_request_irq(unsigned irq, mica_irq_handler_t fn, void *arg);
static inline int mica_unmask_irq(unsigned irq)
{
    return PRT_HwiEnable(irq) == OS_OK ? 0 : -EIO;
}

/* Xen uses event-channel hypercalls; no non-Xen trigger/release is claimed. */
static inline int mica_trigger_irq(unsigned irq)
{
    (void)irq;
    return -EOPNOTSUPP;
}

static inline int mica_sync_release_irq(unsigned irq, mica_deadline_t deadline)
{
    (void)irq;
    (void)deadline;
    return -EOPNOTSUPP;
}
#endif

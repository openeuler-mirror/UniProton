/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H
#include "prt_typedef.h"

#include <stddef.h>
#include <linux/printk.h>
#include <linux/slab.h>

static inline void might_fault(void) { }

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:    the type of the container struct this is embedded in.
 * @member:    the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                \
    void *__mptr = (void *)(ptr);                    \
    ((type *)(__mptr - offsetof(type, member))); })

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);

#endif
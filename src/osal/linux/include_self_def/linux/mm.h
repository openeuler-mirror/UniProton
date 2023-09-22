/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <linux/slab.h>

struct page {
    unsigned long flags; /* Atomic flags, some possibly
                          * updated asynchronously */
};

struct page *vmalloc_to_page(const void *addr);

#endif
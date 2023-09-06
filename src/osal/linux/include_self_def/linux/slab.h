/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SLAB_H
#define _LINUX_SLAB_H

#include <linux/err.h>
#include <linux/types.h>
#include <linux/gfp.h>
#include "prt_module.h"
#include "prt_mem.h"

void kfree(const void * objp);
void *kmalloc(size_t size, gfp_t flags);

#endif
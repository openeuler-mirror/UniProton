/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_VERSION_H
#define _LINUX_VERSION_H

#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/skbuff.h>
#include <linux/jiffies.h>
#include <linux/if_ether.h>

#define likely
#define unlikely

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + ((c) > 255 ? 255 : (c)))
#endif

#define LINUX_VERSION_CODE KERNEL_VERSION(5, 10, 0)

#endif

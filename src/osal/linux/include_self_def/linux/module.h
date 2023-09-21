/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <linux/types.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/export.h>
#include "prt_typedef.h"

// empty struct, has no use
struct module {
    int empty;
};

void module_put(struct module *module);
bool try_module_get(struct module *module);

#endif /* _LINUX_MODULE_H */
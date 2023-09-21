#ifndef _LINUX_VMALLOC_H
#define _LINUX_VMALLOC_H

#include "prt_module.h"
#include "prt_mem.h"

void vfree(const void *addr);
void *vmalloc(unsigned long size);

#endif
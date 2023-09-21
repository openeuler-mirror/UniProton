#include <linux/vmalloc.h>

void *vmalloc(unsigned long size)
{
    return PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, size);
}
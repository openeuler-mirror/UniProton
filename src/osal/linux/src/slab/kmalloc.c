#include <linux/slab.h>

void *kmalloc(size_t size, gfp_t flags)
{
    (void)flags;
    return PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, size);
}
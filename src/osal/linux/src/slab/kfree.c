#include <linux/slab.h>

void kfree(const void * objp)
{
    PRT_MemFree(OS_MID_APP, objp);
}
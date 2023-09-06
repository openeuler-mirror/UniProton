#include <linux/vmalloc.h>

void vfree(const void *addr)
{
    PRT_MemFree(OS_MID_APP, addr);
}
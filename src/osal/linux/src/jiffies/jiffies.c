#include "linux/jiffies.h"
#include "prt_sys_external.h"

unsigned int get_HZ(void)
{
    return g_tickModInfo.tickPerSecond;
}
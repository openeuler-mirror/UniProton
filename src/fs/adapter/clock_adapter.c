#include <nuttx/clock.h>
#include <time.h>
#include "prt_posix_internal.h"

clock_t clock_systime_ticks(void)
{
    return (clock_t)PRT_TickGetCount();
}
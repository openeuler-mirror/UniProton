#include <stdarg.h>
#include "prt_typedef.h"
#include "securec.h"
/* Stateless builtin ABI; no old Xen owner headers or early domain init. */
extern int HYPERVISOR_console_io(int command, int count, char *buffer);
#define CONSOLEIO_write 0

#define OS_MAX_SHOW_LEN 0x200

int TestPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    
    len = vsnprintf_s(buff, OS_MAX_SHOW_LEN, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }

#ifdef OS_OPTION_XEN_CONSOLE_IO
    /* You need to turn on CONFIG_DEBUG in xen image to enable direct console print */
    (void)HYPERVISOR_console_io(CONSOLEIO_write, len, buff);
#else
    /* No BSP output without Xen console I/O; libc/shell use libmica TTY. */
#endif
    
    return OS_OK;
}

U32 PRT_Printf(const char *format, ...)
{
    va_list vaList;
    S32 count;
    
    va_start(vaList, format);
    count = TestPrintf(format, vaList);
    va_end(vaList);
    
    return count;
}

U32 PRT_PrintfInit(void)
{
    return OS_OK;
}

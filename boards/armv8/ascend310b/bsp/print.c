#include <stdarg.h>
#include "prt_typedef.h"
#include "cpu_config.h"
#include "securec.h"
#include "pl011.h"
#include "ymodem.h"

#define WRITE_UINT32(value, addr) {          \
    OS_EMBED_ASM("DSB SY");                  \
    *(volatile U32 *)(addr) = (U32)(value);  \
}

#define GET_UINT32(addr) ({                  \
    U32 r = *(volatile U32 *)(addr);         \
    OS_EMBED_ASM("DSB SY"); r;               \
})

typedef U32 (*PrintFunc)(const char *format, va_list vaList);
#define OS_MAX_SHOW_LEN 0x200

#ifdef GUEST_OS
static void TestPutc(unsigned char ch)
{
    /* 如果当前串口在传输，关闭打印 */
    if (YMODEM_IsEnable()) {
        return;
    }
    UartPutChar(ch);
    if (ch == '\n') {
        UartPutChar('\r');
    }
}
#else
void uart_poll_send(unsigned char ch)
{
    volatile int time = 100;
    *(unsigned long long int *)(UART_BASE_ADDR + UART_DR) = ch;
    while(time--);
}

void TestPutc(unsigned char ch)
{
    uart_poll_send(ch);
    if(ch == '\n') {
        uart_poll_send('\r');
    }
}
#endif
int TestPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    char *str = buff;

    len = vsnprintf_s(buff, OS_MAX_SHOW_LEN, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }

    while (*str != '\0') {
        TestPutc(*str);
        str++;
    }

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

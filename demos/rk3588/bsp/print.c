#include <stdarg.h>
#include "prt_typedef.h"
#include "cpu_config.h"
#include "securec.h"

#define OS_MAX_SHOW_LEN 0x200

U32 PRT_UartInit(void)
{
    return OS_FAIL;
}

void uart_poll_send(unsigned char ch)
{
    /* 暂不使用uart，先直接写串口寄存器地址，启用MMU后速度加快，延时相应的增加 */
    volatile int time = 25000;
    *(unsigned int *)UART_BASE_ADDR = ch;
    while (time--);
}

void TestPutc(unsigned char ch)
{
    uart_poll_send(ch);
    if (ch == '\n') {
        uart_poll_send('\r');
    }
}

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

U32 PRT_PrintfInit()
{
    U32 ret;

    ret = PRT_UartInit();
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

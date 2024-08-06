#include <stdarg.h>
#include <stdio.h>
#include "securec.h"
#include "print.h"

#define OS_MAX_SHOW_LEN 256

extern unsigned long long __os_print_start;
extern unsigned long long __os_print_end;

unsigned long long g_printAddrStart = (unsigned long long)&__os_print_start;
unsigned long long g_printAddrEnd = (unsigned long long)&__os_print_end;
unsigned long long g_printAddr;
PrintHead *g_printHead;

void PrintInit(void)
{
    unsigned long long size = g_printAddrEnd - g_printAddrStart;
    g_printHead = (PrintHead *)g_printAddrStart;
    g_printAddr = g_printAddrStart + sizeof(PrintHead);
    g_printHead->size = size - sizeof(PrintHead);
    g_printHead->head = 0;
    g_printHead->tail = 0;
    g_printHead->stop = 0;
}

void TestPutc(unsigned char ch)
{
    *(unsigned long long *)(g_printAddr + g_printHead->tail) = ch;
    g_printHead->tail = (g_printHead->tail + 1) % g_printHead->size;
    if (ch == '\n') {
        *(unsigned long long *)(g_printAddr + g_printHead->tail) = '\r';
        g_printHead->tail = (g_printHead->tail + 1) % g_printHead->size;
    }
}

extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int Test_Printf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    char *str = buff;

    len = vsnprintf(buff, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }

    while (*str != '\0') {
        TestPutc(*str);
        str++;
    }

    return 0;
}

int printf(const char *format, ...)
{
    int len = 0;
    va_list vaList;

    va_start(vaList, format);
    len = Test_Printf(format, vaList);
    va_end(vaList);

    return len;
}

#include <stdarg.h>
#include "securec.h"

#define OS_MAX_SHOW_LEN 256

extern unsigned long long __os_print_start;
extern unsigned long long __os_print_end;

unsigned long long g_printAddrStart;
unsigned long long g_printAddrEnd;
unsigned long long g_printAddr = 0xf02400000;

typedef struct {
    unsigned long long size;
    unsigned long long head;
    unsigned long long tail;
} PrintHead;

PrintHead *g_printHead;
unsigned long long g_printaddr;

void MyPrint(unsigned long long val);

void PrintInit()
{
    g_printAddrStart = (unsigned long long)&__os_print_start;
    g_printAddrEnd = (unsigned long long)&__os_print_end;
    memset_s((void *)g_printAddrStart, 0x100000, 0, 0x100000);

    g_printHead = (PrintHead *)g_printAddrStart;
    g_printaddr = g_printAddrStart + sizeof(PrintHead);
    g_printHead->size = 0x100000 - sizeof(PrintHead);
    g_printHead->head = 0;
    g_printHead->tail = 0;
}

void MyPrint(unsigned long long val)
{
    *(unsigned long long *)g_printAddr = val;
    g_printAddr += 8;
    if (g_printAddrStart == g_printAddrEnd) {
        g_printAddrStart = (unsigned long long)&__os_print_start;
    }
}

void TestPutc(unsigned char ch)
{
    *(unsigned long long *)(g_printaddr + g_printHead->tail) = ch;
    g_printHead->tail = (g_printHead->tail + 1) % g_printHead->size;
    if (ch == '\n') {
        *(unsigned long long *)(g_printaddr + g_printHead->tail) = '\r';
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
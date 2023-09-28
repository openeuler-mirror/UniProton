#include <stdarg.h>
#include "prt_typedef.h"
#include "cpu_config.h"
#include "securec.h"

#define WRITE_UINT32(value, addr) {          \
    OS_EMBED_ASM("DSB SY");                  \
    *(volatile U32 *)(addr) = (U32)(value);  \
}

#define GET_UINT32(addr) ({                  \
    U32 r = *(volatile U32 *)(addr);         \
    OS_EMBED_ASM("DSB SY"); r;               \
})

#define UART_DR                  0x0  /* data register */
#define UART_IBRD                0x24 /* integer baudrate register */
#define UART_FBRD                0x28 /* decimal baudrate register */
#define UART_LCR_H               0x2C /* line control register */
#define UART_CR                  0x30 /* control register */

#define UART_CR_TX_EN            (0x01 << 8)
#define UART_CR_EN               (0x01 << 0)

#define UART_LCR_H_FIFO_EN       (0x01 << 4)
#define UART_LCR_H_8_BIT         (0x03 << 5)

#define PL011_CLK_DIV            16U
#define PL011_NUM_8              8U
#define CONSOLE_UART_BAUDRATE    115200

typedef U32 (*PrintFunc)(const char *format, va_list vaList);
#define OS_MAX_SHOW_LEN 0x200

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

#if defined(GUEST_OS)

static inline void UartSetBaudrate(uintptr_t regBase)
{
    U32 baudRate;
    U32 divider;
    U32 remainder;
    U32 fraction;

    baudRate  = PL011_CLK_DIV * CONSOLE_UART_BAUDRATE;
    divider   = UART_CLK_INPUT / baudRate;
    remainder = UART_CLK_INPUT % baudRate;
    baudRate  = (PL011_NUM_8 * remainder) / CONSOLE_UART_BAUDRATE;
    fraction  = (baudRate >> 1) + (baudRate & 1);

    WRITE_UINT32(divider, regBase + UART_IBRD);
    WRITE_UINT32(fraction, regBase + UART_FBRD);
}

static void UartEarlyInit(void)
{
    /* First, disable everything */
    WRITE_UINT32(0x0, UART_BASE_ADDR + UART_CR);

    /* set Scale factor of baud rate */
    UartSetBaudrate(UART_BASE_ADDR);

    /* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled. */
    WRITE_UINT32(UART_LCR_H_8_BIT | UART_LCR_H_FIFO_EN, UART_BASE_ADDR + UART_LCR_H);

    /* enable the UART */
    WRITE_UINT32(UART_CR_EN | UART_CR_TX_EN, UART_BASE_ADDR + UART_CR);
}

#endif

void PRT_UartInit(void)
{
#if defined(GUEST_OS)
    /* GPIO pin multiplexing */
    WRITE_UINT32(0, GPIO_UTXD2_ADDR);
    WRITE_UINT32(0, GPIO_URXD2_ADDR);

    UartEarlyInit();
#endif
}
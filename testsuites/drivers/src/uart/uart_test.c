#include "arm_internal.h"

int uart_test()
{
    arm_earlyserialinit();
    arm_serialinit();
    arm_lowputc('H');
    arm_lowputs("ello Word!\n");
    char get = up_putc('h');
    if (get != 'h') {
        return 1;
    }
    return 0;
}
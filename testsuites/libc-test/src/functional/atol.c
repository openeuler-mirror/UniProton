#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    char *orginalString;
    long value;
} t[] = {
    {"2147483647", 2147483647},
    {"-2147483647", -2147483647},
    {"0.04294967295", 0},
    {"Z", 0},
    {"0F5F", 0},
    {"0xz", 0},
    {"0x1234", 0},
    {"  15437", 15437},
    {"  1", 1},
};

int atol_test(void)
{
    int i;
    long testValue;

    for (i = 0; i < length(t); i++) {
        testValue = atol(t[i].orginalString);
        if (testValue != t[i].value)
        {
            printf("atol(\"%s\") want %ld got %ld\n", t[i].orginalString, t[i].value, testValue);
            t_status = 1;
        }


    }

    /* 边界测试 */
    if (sizeof(long) == 8)
    {
        testValue = atol("9223372036854775807");
        if (testValue != 9223372036854775807LL)
            t_error("atol(\"%s\") want %lld got %lld\n", "9223372036854775807", 9223372036854775807LL, testValue);

        testValue = atol("-9223372036854775807");
        if (testValue != -9223372036854775807LL)
            t_error("atol(\"%s\") want %lld got %lld\n", "-9223372036854775807", -9223372036854775807LL, testValue);
    }

    return t_status;
}


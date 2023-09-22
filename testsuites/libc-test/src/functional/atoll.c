#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    char *orginalString;
    long long value;
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
    {"9223372036854775807", 9223372036854775807LL},
    {"-9223372036854775807", -9223372036854775807LL},
};

int atoll_test(void)
{
    int i;
    long long testValue;

    for (i = 0; i < length(t); i++) {
        testValue = atoll(t[i].orginalString);

        if (testValue != t[i].value)
        {
            t_status = 1;
            printf("atoll(\"%s\") want %lld got %lld\n", t[i].orginalString, t[i].value, testValue);
        }
    }
    return t_status;
}


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    char *orginalString;
    int value;
} t[] = {
    {"2147483647", 2147483647},
    {"-2147483647", 2147483649},
    {"0.04294967295", 0},
    {"Z", 0},
    {"0F5F", 0},
    {"0xz", 0},
    {"0x1234", 0},
    {"  15437", 15437},
    {"  1", 1},
    {"2147483647", 2147483647},
    {"-2147483648", -2147483648},
};

int atoi_test(void)
{
    int i;
    int testValue;

    for (i = 0; i < length(t); i++) {
        testValue = atoi(t[i].orginalString);
        if (testValue != t[i].value)
        {
            printf("atoi(\"%s\") want %d got %d\n", t[i].orginalString, t[i].value, testValue);
            t_status = 1;
        }
    }
    return t_status;
}

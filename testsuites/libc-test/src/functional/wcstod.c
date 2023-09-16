#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <math.h>
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    char *orginalString;
    double f;
    char *endString;
} t[] = {
    {"0End", 0.0, "0End"},
    {"00.00End", 0.0, "00.00End"},
    {"136.31e-2End", 0.0, "136.31e-2End"}, /* 十进制浮点值 */
    {"0x11a.2cEnd", 0.0, "0x11a.2cEnd"}, /* 十六进制测试 */
    {"-inFinityEnd", 0.0, "-inFinityEnd"}, /* Infinity */
    {"NaN11End", 0.0, "NaN11End"}, /* NaN */
    {"-.00000", 0.0, "-.00000"},

};

int wcstod_test(void)
{
    int i;
    double x;
    char *end;

    for (i = 0; i < length(t); i++) {
        x = wcstod((wchar_t *)t[i].orginalString, (wchar_t **)&end);
        if (x != t[i].f)
        {
            printf("wcstod(\"%s\") want %f got %f\n", t[i].orginalString, t[i].f, x);
            t_status = 1;
        }
        else if (strcmp(t[i].endString, end) != 0)
        {
            printf("wcstod(\"%s\") want str %s got str %s\n", t[i].orginalString, t[i].endString, end);
            t_status = 1;
        }
    }

    return t_status;
}
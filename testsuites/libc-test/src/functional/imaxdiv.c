#include "test.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

static struct {
    intmax_t x, y, div, mod;
} t[] = {
2147483647, 2147483646, 1, 1,
2147483647, 2147483647, 1, 0,
2147483647, 536870908, 4, 15,
-2147483648, 2147483647, -1, -1,
-2147483647, -1, 2147483647, 0,
2147483647, 1, 2147483647, 0,

/* 边界测试 */
9223372036854775807LL, 3, 3074457345618258602LL, 1,
9223372036854775807LL, 2147483647, 4294967298LL, 1,
-9223372036854775807LL, -3, 3074457345618258602LL, -1

};

int imaxdiv_test(void)
{
    int i;
    intmax_t x, y;
    imaxdiv_t divResult;

    for (i = 0; i < sizeof t/sizeof *t; i++) {
        x = t[i].x;
        y = t[i].y;
        divResult = imaxdiv(x, y);

        if (divResult.quot != t[i].div || divResult.rem != t[i].mod)
        {
            printf("imaxdiv %lld/%lld : want div %lld got div %lld;", x, y, t[i].div, divResult.quot);
            printf("want mode %lld got %lld\n", t[i].mod, divResult.rem);
            t_status = 1;
        }
    }

    return t_status;
}

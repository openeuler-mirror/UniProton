#include "stdlib.h"
#include "inttypes.h"
#include "test.h"

int imaxabs_test(void)
{

    intmax_t absoluteValue;

    absoluteValue = imaxabs(-0);
    if (0 != absoluteValue)
        t_error("imaxabs(\"%s\") got %ld\n", "-0", absoluteValue);

    absoluteValue = imaxabs(-1);
    if (1 != absoluteValue)
        t_error("imaxabs(\"%s\") got %ld\n", "-1", absoluteValue);

    absoluteValue = imaxabs(1);
    if (1 != absoluteValue)
        t_error("imaxabs(\"%s\") got %ld\n", "1", absoluteValue);

    /* 边界测试 */
    if (sizeof(intmax_t) == 4) {
        absoluteValue = imaxabs(2147483647);
        if (2147483647L != absoluteValue)
            t_error("uncaught overflow imaxabs(\"%s\") got %ld\n", "2147483647", 2147483647L);

        absoluteValue = imaxabs(-2147483648);
        if (-2147483648L != absoluteValue)
            t_error("uncaught overflow imaxabs(\"%s\") got %ld\n", "-2147483648", -2147483648L);
    } else if (sizeof(intmax_t) == 8) {
        absoluteValue = imaxabs(9223372036854775807LL);
        if (9223372036854775807LL != absoluteValue)
            t_error("uncaught overflow imaxabs(\"%s\") got %ld\n", "9223372036854775807", absoluteValue);

        absoluteValue = imaxabs(-9223372036854775807LL);
        if (9223372036854775807LL != absoluteValue)
            t_error("uncaught overflow imaxabs(\"%s\") got %ld\n", "-9223372036854775808", absoluteValue);
    } else {
        t_error("sizeof(intmax_t) == %d, not implemented\n", (int)sizeof(intmax_t));
    }

    return t_status;
}
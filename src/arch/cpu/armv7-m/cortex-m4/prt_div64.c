/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-07-25
 * Description: 64位除法文件。
 */
#include "prt_typedef.h"
#include "prt_lib_external.h"
#include "prt_attr_external.h"

#define OS_32_BITS_COUNT 32

/*
 * 描述：前导0个数获取
 */
OS_SEC_TEXT U32 OsleadingZeroCount(U64 data)
{
    U32 high = (U32)OS_GET_64BIT_HIGH_32BIT(data);
    U32 low = (U32)OS_GET_64BIT_LOW_32BIT(data);
    U32 count = 0;

    if (high == 0) {
        OS_EMBED_ASM("CLZ %0, %1" : "=r"(count) : "r"(low));
        count += OS_32_BITS_COUNT;
    } else {
        OS_EMBED_ASM("CLZ %0, %1" : "=r"(count) : "r"(high));
    }

    return count;
}

OS_SEC_TEXT void OsU64Div(U64 dividend, U64 divisor, U64 *quotient, U64 *remainder)
{
    U32 alignShift;
    U32 i;
    U64 tmpDivisor;
    U64 tmpRemainder;
    U64 tmpQuotient = 0;

    if (divisor == 0) {
        return;                  // 除数为0返回1
    }

    if (dividend < divisor) {
        /* 被除数小于除数,商为0,余数即被除数 */
        *remainder = dividend;
        *quotient = 0;
        return;
    }

    alignShift = OsleadingZeroCount(divisor) - OsleadingZeroCount(dividend);
    tmpDivisor = divisor << alignShift;
    tmpRemainder = dividend;

    /* 竖式除法, 类大数相除 */
    for (i = 0; i <= alignShift; i++) {
        tmpQuotient <<= 1;
        if (tmpRemainder >= tmpDivisor) {
            tmpRemainder -= tmpDivisor;
            tmpQuotient += 1;
        }
        tmpDivisor >>= 1;
    }

    *quotient = tmpQuotient;
    *remainder = tmpRemainder;

    return;
}

OS_SEC_TEXT U64 OsU64DivGetQuotient(U64 dividend, U64 divisor)
{
    U64 quotient = 0;
    U64 remainder;

    OsU64Div(dividend, divisor, &quotient, &remainder);

    return quotient;
}

OS_SEC_TEXT U64 OsU64DivGetRemainder(U64 dividend, U64 divisor)
{
    U64 quotient;
    U64 remainder = 0;

    OsU64Div(dividend, divisor, &quotient, &remainder);

    return remainder;
}
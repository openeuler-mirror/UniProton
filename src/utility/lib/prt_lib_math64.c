/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 提供64位加减操作
 */
#include "prt_lib_external.h"
#include "prt_attr_external.h"

/*
 * 描述：64位加法
 */
OS_SEC_ALW_INLINE INLINE void OsAdd64X(U32 oldLow, U32 oldHigh, U32 *low, U32 *high)
{
    if (*low > OS_MAX_U32 - oldLow) {
        *high += (oldHigh + 1);
    } else {
        *high += oldHigh;
    }
    *low += oldLow;

    return;
}

OS_SEC_L4_TEXT void OsAdd64(U32 *low, U32 *high, U32 oldLow, U32 oldHigh)
{
    OsAdd64X(oldLow, oldHigh, low, high);
}

/*
 * 描述：64位减法
 */
OS_SEC_ALW_INLINE INLINE void OsSub64X(U32 oldLow, U32 oldHigh, U32 *low, U32 *high)
{
    if (*low >= oldLow) {
        *high -= oldHigh;
    } else {
        *high -= (oldHigh + 1);
    }
    *low -= oldLow;

    return;
}

OS_SEC_L4_TEXT void OsSub64(U32 *low, U32 *high, U32 oldLow, U32 oldHigh)
{
    OsSub64X(oldLow, oldHigh, low, high);
}

OS_SEC_L4_TEXT U32 OsGetLmb1(U32 value)
{
    int i;
    U32 max = 0;

    for (i = 31; i >= 0; i--) {
        if (((1U << (U32)i) & value) != 0) {
            max = (U32)i;
            break;
        }
    }

    return (31U - max);
}


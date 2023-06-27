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
 * Create: 2009-12-22
 * Description: 软定时器模块的内部头文件
 */
#ifndef PRT_SWTMR_EXTERNAL_H
#define PRT_SWTMR_EXTERNAL_H

#include "prt_timer_external.h"
#include "prt_list_external.h"

/* 最大支持软件定时器个数 */
extern U32 g_swTmrMaxNum;
OS_SEC_ALW_INLINE INLINE U32 OsSwTmrMaxNum(void)
{
    return g_swTmrMaxNum;
}
#define OS_TIMER_MAX_NUM ((TIMER_TYPE_SWTMR << 28) + OsSwTmrMaxNum())
#define OS_SWTMR_MIN_NUM (TIMER_TYPE_SWTMR << 28)
/*
 * 模块间宏定义
 */
#define OS_SWTMR_INDEX_2_ID(index) (((U32)(index)) + OS_SWTMR_MIN_NUM)
#define OS_SWTMR_ID_2_INDEX(timerId) ((timerId) - OS_SWTMR_MIN_NUM)

#define SWTMR_CREATE_DEL_LOCK()
#define SWTMR_CREATE_DEL_UNLOCK()

/* 软件定时器结构定义 */
struct TagSwTmrCtrl {
    /* 指向前一个定时器 */
    struct TagSwTmrCtrl *prev;
    /* 指向下一个定时器 */
    struct TagSwTmrCtrl *next;
    /* 定时器状态 */
    U8 state;
    /* 定时器类型 */
    U8 mode;
    /* 软件定时器序号 */
    U16 swtmrIndex;
    /* 定时器SortLink属性，低位为rollNum，高位为sortIndex */
    U32 idxRollNum;
    /* 定时器超时时间 */
    U32 interval;
    /* 定时器超时次数 */
    U8 overrun;
    /* 定时器用户参数1 */
    U32 arg1;
    /* 定时器用户参数2 */
    U32 arg2;
    /* 定时器用户参数3 */
    U32 arg3;
    /* 定时器用户参数4 */
    U32 arg4;
    /* 定时器超时处理函数 */
    TmrProcFunc handler;

}; /* 定时器数据类型 */

struct TagSwTmrSortLinkAttr {
    /* Tick游标位置 */
    U16 cursor;
    /* 保留数据 */
    U16 unused;
    /* 软件定时器指针数组 */
    struct TagListObject *sortLink;
};

/*
 * 模块间typedef声明
 */
/*
 * 模块间全局变量声明
 */
/* 定时器内存空间首地址 */
extern struct TagSwTmrCtrl *g_swtmrCbArray;

/*
 * 模块间函数声明
 */
/*
 * Function    : OsSwTmrQuery
 * Description : 查询软件定时器剩余超时时间
 * Input       : tmrHandle  --- 定时器句柄
 *               expireTime  --- 定时器的剩余的超时时间，单位ms
 * Output      : None
 * Return      : 成功时返回OS_OK，失败时返回错误码
 */
extern U32 OsSwTmrQuery(TimerHandle tmrHandle, U32 *expireTime);

/*
 * 模块间内联函数定义
 */
#endif /* PRT_SWTMR_EXTERNAL_H */

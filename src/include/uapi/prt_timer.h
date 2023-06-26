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
 * Description: 定时器模块的对外头文件。
 */
#ifndef PRT_TIMER_H
#define PRT_TIMER_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * 定时器状态枚举值
 */
enum TimerFlag {
    OS_TIMER_FREE = 1, /* 定时器空闲状态 */
    OS_TIMER_CREATED = 2, /* 定时器没有运行 */
    OS_TIMER_RUNNING = 4, /* 定时器正在运行 */
    OS_TIMER_EXPIRED = 8 /* 定时器已经超时 */
};

/*
 * OS_timer错误码: 指针参数为空。
 *
 * 值: 0x02000d01
 *
 * 解决方案: 查看入参指针是否为空。
 */
#define OS_ERRNO_TIMER_INPUT_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x01)

/*
 * OS_timer错误码: 定时器回调函数为空。
 *
 * 值: 0x02000d02
 *
 * 解决方案: 查看定时器回调函数是否为空。
 */
#define OS_ERRNO_TIMER_PROC_FUNC_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x02)

/*
 * OS_timer错误码: 定时器句柄非法。
 *
 * 值: 0x02000d03
 *
 * 解决方案: 检查输入的定时器句柄是否正确。
 */
#define OS_ERRNO_TIMER_HANDLE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x03)

/*
 * OS_timer错误码: 定时器周期参数非法。
 *
 * 值: 0x02000d04
 *
 * 解决方案: 查看定时器周期参数是否正确。
 */
#define OS_ERRNO_TIMER_INTERVAL_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x04)

/*
 * OS_timer错误码: 定时器工作模式参数非法。
 *
 * 值: 0x02000d05
 *
 * 解决方案: 查看定时器工作模式参数是否正确，参照工作模式配置枚举定义#enum TmrMode。
 */
#define OS_ERRNO_TIMER_MODE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x05)

/*
 * OS_timer错误码: 定时器类型参数非法。
 *
 * 值: 0x02000d06
 *
 * 解决方案: 查看定时器类型参数是否正确，参照类型配置枚举定义#enum TimerType。
 */
#define OS_ERRNO_TIMER_TYPE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x06)

/*
 * 软件定时器错误码：软件定时器组的最大定时器个数为零。
 *
 * 值: 0x02000d07
 *
 * 解决方案: 软件定时器组的最大定时器个数必须大于零。
 */
#define OS_ERRNO_TIMER_NUM_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x07)

/*
 * 软件定时器错误码：定时器未初始化或定时器组未创建。
 *
 * 值: 0x02000d08
 *
 * 解决方案: 只有在定时器初始化且定时器组已经创建的情况下才能使用定时器。
 */
#define OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x08)

/*
 * 软件定时器错误码:定时器设置的定时周期转化为Tick数后不为整数。
 *
 * 值: 0x02000d09
 *
 * 解决方案: 请确保设置的定时周期是Tick的整数倍。
 */
#define OS_ERRNO_SWTMR_INTERVAL_NOT_SUITED OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x09)

/*
 * 软件定时器错误码:达到最大支持定时器数目。
 *
 * 值: 0x02000d0a
 *
 * 解决方案: 达到最大支持定时器个数，不能再创建软件定时器。
 */
#define OS_ERRNO_SWTMR_MAXSIZE OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0a)

/*
 * 软件定时器错误码:定时器未创建。
 *
 * 值: 0x02000d0b
 *
 * 解决方案: 创建定时器后再使用。
 */
#define OS_ERRNO_SWTMR_NOT_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0b)

/*
 * 软件定时器错误码:定时器已经处于未启动状态。
 *
 * 值: 0x02000d0c
 *
 * 解决方案: 定时器处于未启动状态，不能进行一些操作，请检查操作的合法性。
 */
#define OS_ERRNO_SWTMR_UNSTART OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0c)

/*
 * 软件定时器错误码:初始化内存不足。
 *
 * 值: 0x02000d0d
 *
 * 解决方案: 请适当增加系统默认FSC分区大小。
 */
#define OS_ERRNO_SWTMR_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0d)

/*
 * 软件定时器错误码:TICK没有初始化。
 *
 * 值: 0x02000d0e
 *
 * 解决方案: 初始化Tick。
 */
#define OS_ERRNO_TICK_NOT_INIT OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0e)

/*
 * 软件定时器错误码:软件定时器定时周期过大，转换为Tick数超过0xFFFFFFFF。
 *
 * 值: 0x02000d0f
 *
 * 解决方案: 请确保传入的定时周期转化为Tick数后不大于0xFFFFFFFF。
 */
#define OS_ERRNO_SWTMR_INTERVAL_OVERFLOW OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x0f)

/*
 * 软件定时器错误码:Tick定时器组已创建。
 *
 * 值: 0x02000d10
 *
 * 解决方案: 不能重复创建定时器组。
 */
#define OS_ERRNO_TIMER_TICKGROUP_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x10)

/*
 * 软件定时器错误码:创建软件定时器时传入定时器组组号非法。
 *
 * 值: 0x02000d11
 *
 * 解决方案: 确保传入的定时器组号是通过使用定时器组创建接口得到。
 */
#define OS_ERRNO_TIMERGROUP_ID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x11)

/*
 * 软件定时器错误码：软件定时器组的最大定时器个数大于0xffff。
 *
 * 值: 0x02000d12
 *
 * 解决方案: 确保软件定时器组的最大定时器个数小于等于0xffff。
 */
#define OS_ERRNO_TIMER_NUM_TOO_LARGE OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x12)

/*
 * 软件定时器错误码:返回指针参数为空。
 *
 * 值: 0x02000d13
 *
 * 解决方案: 输入有效的指针参数。
 */
#define OS_ERRNO_SWTMR_RET_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TIMER, 0x13)

/*
 * 定时器句柄定义
 */
typedef U32 TimerHandle;

/*
 * 定时器组句柄定义
 */
typedef U32 TimerGroupId;

/*
 * 定时器回调函数定义，带5个参数、返回值类型为void的函数指针类型，其中参数tmrHandle在核内硬件定时器、
 * 全局硬件定时器和软件定时器中分别表示逻辑ID、物理ID和逻辑ID。
 */
typedef void (*TmrProcFunc)(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4);

/*
 * 定时器组时钟源类型枚举定义
 */
enum TimerGrpSrcType {
    OS_TIMER_GRP_SRC_HARDWARE, /* 硬件私有时钟源 */
    OS_TIMER_GRP_SRC_HARDWARE_SHARED, /* 硬件共享时钟源 */
    OS_TIMER_GRP_SRC_EXTERN, /* 外部时钟源 */
    OS_TIMER_GRP_SRC_TICK, /* TICK时钟源 */
    OS_TIMER_GRP_SRC_INVALID
};

/*
 * 定时器组用户配置结构体
 */
struct TmrGrpUserCfg {
    /* 时钟源类型 */
    enum TimerGrpSrcType tmrGrpSrcType;
    /* 时间轮的刻度，即每刻度多少个us */
    U32 perStep;
    /* 最大定时器个数 */
    U32 maxTimerNum;
    /* 时间轮长度必须是2的N次方，此处配置N的大小 */
    U32 wheelLen2Power;
};

/*
 * 定时器工作模式枚举定义
 */
enum TmrMode {
    OS_TIMER_LOOP, /* 定时器周期触发模式 */
    OS_TIMER_ONCE, /* 定时器单次触发模式 */
    OS_TIMER_INVALID_MODE
};

/*
 * 定时器类型枚举定义
 */
enum TimerType {
    OS_TIMER_HARDWARE, /* 硬件定时器(核内私有硬件定时器) */
    OS_TIMER_SOFTWARE, /* 软件定时器(核内私有软件定时器) */
    OS_TIMER_SOFTWARE_SHARED, /* 共享软件定时器，目前不支持 */
    OS_TIMER_INVALID_TYPE
};

/*
 * 定时器创建参数结构体定义
 */
struct TimerCreatePara {
    /* 定时器创建模块ID,当前未使用，忽略 */
    U32 moduleId;
    /* 定时器类型 */
    enum TimerType type;
    /* 定时器工作模式 */
    enum TmrMode mode;
    /* 定时器周期(单次指定时器响应时长)，软件定时器单位是ms，硬件定时器单位是us，高精度定时器单位是ns */
    U32 interval;
    /*
     * 定时器组号，硬件定时器不使用，若创建软件定时器，定时器组ID由OS创建
     */
    U32 timerGroupId;
    /* 定时器硬中断优先级 */
    U16 hwiPrio;
    U16 resv;
    /*
     * 定时器回调函数
     */
    TmrProcFunc callBackFunc;
    /* 定时器用户参数1 */
    U32 arg1;
    /* 定时器用户参数2 */
    U32 arg2;
    /* 定时器用户参数3 */
    U32 arg3;
    /* 定时器用户参数4 */
    U32 arg4;
};

/*
 * 软件定时器信息的结构体类型定义
 */
struct SwTmrInfo {
    /* 定时器状态，三种状态:Free,Created,Running,Expired */
    U8 state;
    /* 保留字段 */
    U8 resved[3];
    /* 定时器类型，两种类型:周期性、一次性 */
    enum TmrMode mode;
    /* 定时器超时间隔 */
    U32 interval;
    /* 定时器离超时剩余的ms数 */
    U32 remainMs;
    /* 定时器超时处理函数 */
    TmrProcFunc handler;
};

/*
 * @brief 创建定时器。
 *
 * @par 描述
 * 创建一个属性为createPara的定时器，返回定时器逻辑ID tmrHandle。
 *
 * @attention
 * <ul>
 * <li>如果用户打开Tick开关则可创建硬件定时器个数少一个。</li>
 * <li>中断处理函数handler的第一个参数是创建的定时器的逻辑编号。</li>
 * <li>定时器创建成功后并不立即开始计数，需显式调用#PRT_TimerStart或者#PRT_TimerRestart启动。</li>
 * <li>对于周期定时模式的定时器，建议用户不要把定时间隔设置的过低，避免一直触发定时器的处理函数。</li>
 * <li>struct TimerCreatePara参数里面的interval元素表示定时器周期，软件定时器单位是ms，
 * 核内硬件定时器、全局硬件定时器单位是us，设置时间间隔的时候请注意适配，过大会出现溢出。</li>
 * </ul>
 *
 * @param createPara [IN]  类型#struct TimerCreatePara *，定时器创建参数
 * @param tmrHandle  [OUT] 类型#TimerHandle *，定时器句柄
 *
 * @retval #OS_OK  0x00000000，定时器创建成功。
 * @retval #其它值，创建失败。
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerDelete
 */
extern U32 PRT_TimerCreate(struct TimerCreatePara *createPara, TimerHandle *tmrHandle);

/*
 * @brief 删除定时器。
 *
 * @par 描述
 * 释放一个定时器逻辑ID为tmrHandle的定时器资源。
 *
 * @attention
 * <ul>
 * <li>硬件定时器删除后将停止计数。</li>
 * <li>删除处于超时状态下的软件定时器时，OS采用延时删除的方式。详细说明请见手册注意事项。</li>
 * </ul>
 *
 * @param mid       [IN]  类型#U32，模块号，当前未使用，忽略
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过PRT_TimerCreate接口获取
 *
 * @retval #OS_OK  0x00000000，定时器删除成功。
 * @retval #其它值，删除失败。
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerCreate
 */
extern U32 PRT_TimerDelete(U32 mid, TimerHandle tmrHandle);

/*
 * @brief 启动定时器。
 *
 * @par 描述
 * 将定时器逻辑ID为tmrHandle的定时器由创建状态变成启动状态。
 *
 * @attention
 * <ul>
 * <li>创建后初次启动定时器时，从设置的值开始计数；如果重复启动或者启动后停止然后再启动，
 * 从重复启动点或者停止点的计数值开始计数。</li>
 * <li>对于全局硬件定时器，无论是第一次启动还是重复启动，下一次启动都从初始值开始计时。</li>
 * <li>对于单次触发模式，触发一次后，可以调用启动接口再次启动该定时器，时间间隔为设置的时间间隔。</li>
 * <li>启动处于超时状态下的软件定时器时，OS采用延时启动的方式。详细说明请见手册注意事项。</li>
 * </ul>
 *
 * @param mid       [IN]  类型#U32，模块号，当前未使用，忽略
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
 *
 * @retval #OS_OK  0x00000000，定时器启动成功。
 * @retval #其它值，启动失败。
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerStop
 */
extern U32 PRT_TimerStart(U32 mid, TimerHandle tmrHandle);

/*
 * @brief 停止定时器。
 *
 * @par 描述
 * 停止定时器逻辑ID为tmrHandle的定时器计数，使定时器由计时状态变成创建状态。
 *
 * @attention
 * <ul>
 * <li>定时器停止后，下一次启动将重新从停止点的计数值开始计数。
 * 但是对于全局硬件定时器，下一次启动从初始值开始计时。</li>
 * <li>不能停止未启动的定时器。</li>
 * <li>停止处于超时状态下的软件定时器时，OS采用延时停止的方式。详细说明请见手册注意事项。</li>
 * <li>核内硬件定时器在停止过程中如果发生超时，则剩余时间为0，但不会响应定时器处理函数。</li>
 * </ul>
 *
 * @param mid       [IN]  类型#U32，模块号，当前未使用，忽略
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
 *
 * @retval #OS_OK  0x00000000，定时器停止成功。
 * @retval #其它值，停止失败。
 *
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerStart
 */
extern U32 PRT_TimerStop(U32 mid, TimerHandle tmrHandle);

/*
 * @brief 重启定时器
 *
 * @par 描述
 * 重启定时器逻辑ID为tmrHandle的定时器计数，对于停止过的定时器，相当于恢复到刚创建时的定时时长开始计时。
 *
 * @attention
 * <ul>
 * <li>重启处于超时状态下的软件定时器时，OS采用延时重启的方式。详细说明请见手册注意事项。</li>
 * </ul>
 *
 * @param mid       [IN]  类型#U32，模块号，当前未使用，忽略
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
 *
 * @retval #OS_OK  0x00000000，定时器重启成功。
 * @retval #其它值，重启失败。
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerStop
 */
extern U32 PRT_TimerRestart(U32 mid, TimerHandle tmrHandle);

/*
 * @brief 定时器剩余超时时间查询
 *
 * @par 描述
 * 输入定时器句柄tmrHandle，获得对应定时器超时剩余时间expireTime。
 *
 * @attention
 * <ul>
 * <li>软件定时器单位毫秒，核内和全局硬件定时器单位微秒。</li>
 * <li>由于OS内部软件定时器采用Tick作为计时单位，硬件定时器采用Cycle作为计时单位，
 * 所以剩余时间转化成ms或us不一定是整数，当转化后的ms或us数不为整数时，返回的剩余时间是该ms或us数取整后+1;
 * 例如转化后ms数为4.2，则最终用户得到的剩余时间是5ms。</li>
 * </ul>
 *
 * @param mid        [IN]  类型#U32，模块号，当前未使用，忽略
 * @param tmrHandle  [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
 * @param expireTime [OUT] 类型#U32 *，定时器的剩余的超时时间，共享和私有硬件定时器单位us，软件定时器单位ms
 *
 * @retval #OS_OK  0x00000000，定时器剩余超时时间查询成功。
 * @retval #其它值，查询失败。
 * @par 依赖
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TimerCreate
 */
extern U32 PRT_TimerQuery(U32 mid, TimerHandle tmrHandle, U32 *expireTime);

/*
 * @brief 获取指定软件定时器的信息。
 *
 * @par 描述
 * 根据指定的定时器ID，获取定时器ID为tmrHandle的定时器的信息info。
 *
 * @attention
 * <ul>
 * <li>由于OS内部采用Tick作为软件定时器的时钟源，所以剩余时间转化成ms不一定是整数，
 * 当转化后的毫秒数不为整数时，返回的剩余时间是该毫秒数取整后+1;
 * 例如转化后毫秒数为4.2，则最终用户得到的剩余时间是5ms。</li>
 * </ul>
 *
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄；
 * @param info      [OUT] 类型#struct SwTmrInfo *，存放定时器信息结构体指针。
 *
 * @retval #SRE_OK  0x00000000，获取指定定时器的信息成功。
 * @retval #其他值  信息获取失败。
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U32 PRT_SwTmrInfoGet(TimerHandle tmrHandle, struct SwTmrInfo *info);

/*
 * @brief 获取指定软件定时器的超时次数。
 *
 * @par 描述
 * 根据指定的定时器ID，获取定时器ID为tmrHandle的超时次数。
 *
 * @attention
 * <ul>
 * <li>软件定时器超时次数最大为255，如果超时次数大于255则指返回255</li>
 * </ul>
 *
 * @param tmrHandle [IN]  类型#TimerHandle，定时器句柄；
 * @param overrun      [OUT] 类型U32 *，存放软件定时器的超时次数。
 *
 * @retval #OS_OK  0x00000000，获取指定定时器的信息成功。
 * @retval #其他值  信息获取失败。
 * <ul><li>prt_timer.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U32 PRT_TimerGetOverrun(U32 mid, TimerHandle tmrHandle, U32 *overrun);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_TIMER_H */

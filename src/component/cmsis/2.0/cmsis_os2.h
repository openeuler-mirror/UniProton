/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2026-06-16
 * Description: CMSIS-RTOS v2 API definitions for UniProton.
 */

#ifndef CMSIS_OS2_H_
#define CMSIS_OS2_H_

/* 编译器无返回函数属性兼容定义。 */
#ifndef __NO_RETURN
#if   defined(__CC_ARM)
#define __NO_RETURN __declspec(noreturn)
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __NO_RETURN __attribute__((__noreturn__))
#elif defined(__GNUC__)
#define __NO_RETURN __attribute__((__noreturn__))
#elif defined(__ICCARM__)
#define __NO_RETURN __noreturn
#else
#define __NO_RETURN
#endif
#endif

#include <stdint.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C"
{
#endif

/* CMSIS-RTOS v2版本信息。 */
typedef struct {
  /* CMSIS-RTOS API版本号。 */
  uint32_t                       api;
  /* UniProton内核适配版本号。 */
  uint32_t                    kernel;
} osVersion_t;

/* 内核状态。 */
typedef enum {
  osKernelInactive        =  0,
  osKernelReady           =  1,
  osKernelRunning         =  2,
  osKernelLocked          =  3,
  osKernelSuspended       =  4,
  osKernelError           = -1,
  osKernelReserved        = 0x7FFFFFFFU
} osKernelState_t;

/* 线程状态。 */
typedef enum {
  osThreadInactive        =  0,
  osThreadReady           =  1,
  osThreadRunning         =  2,
  osThreadBlocked         =  3,
  osThreadTerminated      =  4,
  osThreadError           = -1,
  osThreadReserved        = 0x7FFFFFFF
} osThreadState_t;

/* CMSIS-RTOS v2线程优先级。 */
typedef enum {
  osPriorityNone          =  0,
  osPriorityIdle          =  1,
  osPriorityLow           =  8,
  osPriorityLow1          =  8+1,
  osPriorityLow2          =  8+2,
  osPriorityLow3          =  8+3,
  osPriorityLow4          =  8+4,
  osPriorityLow5          =  8+5,
  osPriorityLow6          =  8+6,
  osPriorityLow7          =  8+7,
  osPriorityBelowNormal   = 16,
  osPriorityBelowNormal1  = 16+1,
  osPriorityBelowNormal2  = 16+2,
  osPriorityBelowNormal3  = 16+3,
  osPriorityBelowNormal4  = 16+4,
  osPriorityBelowNormal5  = 16+5,
  osPriorityBelowNormal6  = 16+6,
  osPriorityBelowNormal7  = 16+7,
  osPriorityNormal        = 24,
  osPriorityNormal1       = 24+1,
  osPriorityNormal2       = 24+2,
  osPriorityNormal3       = 24+3,
  osPriorityNormal4       = 24+4,
  osPriorityNormal5       = 24+5,
  osPriorityNormal6       = 24+6,
  osPriorityNormal7       = 24+7,
  osPriorityAboveNormal   = 32,
  osPriorityAboveNormal1  = 32+1,
  osPriorityAboveNormal2  = 32+2,
  osPriorityAboveNormal3  = 32+3,
  osPriorityAboveNormal4  = 32+4,
  osPriorityAboveNormal5  = 32+5,
  osPriorityAboveNormal6  = 32+6,
  osPriorityAboveNormal7  = 32+7,
  osPriorityHigh          = 40,
  osPriorityHigh1         = 40+1,
  osPriorityHigh2         = 40+2,
  osPriorityHigh3         = 40+3,
  osPriorityHigh4         = 40+4,
  osPriorityHigh5         = 40+5,
  osPriorityHigh6         = 40+6,
  osPriorityHigh7         = 40+7,
  osPriorityRealtime      = 48,
  osPriorityRealtime1     = 48+1,
  osPriorityRealtime2     = 48+2,
  osPriorityRealtime3     = 48+3,
  osPriorityRealtime4     = 48+4,
  osPriorityRealtime5     = 48+5,
  osPriorityRealtime6     = 48+6,
  osPriorityRealtime7     = 48+7,
  osPriorityISR           = 56,
  osPriorityError         = -1,
  osPriorityReserved      = 0x7FFFFFFF
} osPriority_t;

/* 线程入口函数类型。 */
typedef void (*osThreadFunc_t) (void *argument);

/* 定时器回调函数类型。 */
typedef void (*osTimerFunc_t) (void *argument);

/* 定时器类型。 */
typedef enum {
  osTimerOnce               = 0,
  osTimerPeriodic           = 1
} osTimerType_t;

/* CMSIS-RTOS v2永久等待超时值。 */
#define osWaitForever         0xFFFFFFFFU

/* 事件标志和线程标志等待选项。 */
#define osFlagsWaitAny        0x00000000U
#define osFlagsWaitAll        0x00000001U
#define osFlagsNoClear        0x00000002U

/* 事件标志和线程标志错误返回值。 */
#define osFlagsError          0x80000000U
#define osFlagsErrorUnknown   0xFFFFFFFFU
#define osFlagsErrorTimeout   0xFFFFFFFEU
#define osFlagsErrorResource  0xFFFFFFFDU
#define osFlagsErrorParameter 0xFFFFFFFCU
#define osFlagsErrorISR       0xFFFFFFFAU

/* 线程属性：创建为分离线程。 */
#define osThreadDetached      0x00000000U
/* 线程属性：创建为可join线程。 */
#define osThreadJoinable      0x00000001U

/* 互斥锁属性：递归互斥锁。 */
#define osMutexRecursive      0x00000001U
/* 互斥锁属性：优先级继承。 */
#define osMutexPrioInherit    0x00000002U
/* 互斥锁属性：鲁棒互斥锁。 */
#define osMutexRobust         0x00000008U

/* CMSIS-RTOS v2接口返回状态。 */
typedef enum {
  osOK                      =  0,
  osError                   = -1,
  osErrorTimeout            = -2,
  osErrorResource           = -3,
  osErrorParameter          = -4,
  osErrorNoMemory           = -5,
  osErrorISR                = -6,
  osStatusReserved          = 0x7FFFFFFF
} osStatus_t;

/* 线程对象句柄。 */
typedef void *osThreadId_t;

/* 定时器对象句柄。 */
typedef void *osTimerId_t;

/* 事件标志对象句柄。 */
typedef void *osEventFlagsId_t;

/* 互斥锁对象句柄。 */
typedef void *osMutexId_t;

/* 信号量对象句柄。 */
typedef void *osSemaphoreId_t;

/* 内存池对象句柄。 */
typedef void *osMemoryPoolId_t;

/* 消息队列对象句柄。 */
typedef void *osMessageQueueId_t;

#ifndef TZ_MODULEID_T
#define TZ_MODULEID_T

/* TrustZone模块标识类型。 */
typedef uint32_t TZ_ModuleId_t;
#endif

/* 线程属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 线程属性位。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
  /* 用户提供的栈内存地址，当前适配层保留。 */
  void                   *stack_mem;
  /* 线程栈大小，单位为字节。 */
  uint32_t                stack_size;
  /* 线程初始优先级。 */
  osPriority_t              priority;
  /* TrustZone模块标识。 */
  TZ_ModuleId_t            tz_module;
  /* 保留字段，必须为0。 */
  uint32_t                  reserved;
} osThreadAttr_t;

/* 定时器属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 属性位，当前适配层保留。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
} osTimerAttr_t;

/* 事件标志属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 属性位，当前适配层保留。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
} osEventFlagsAttr_t;

/* 互斥锁属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 互斥锁属性位。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
} osMutexAttr_t;

/* 信号量属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 属性位，当前适配层保留。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
} osSemaphoreAttr_t;

/* 内存池属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 属性位，当前适配层保留。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
  /* 用户提供的内存池地址，当前适配层保留。 */
  void                      *mp_mem;
  /* 用户提供的内存池大小，当前适配层保留。 */
  uint32_t                   mp_size;
} osMemoryPoolAttr_t;

/* 消息队列属性结构体。 */
typedef struct {
  /* 对象名称。 */
  const char                   *name;
  /* 属性位，当前适配层保留。 */
  uint32_t                 attr_bits;
  /* 用户提供的控制块内存地址，当前适配层保留。 */
  void                      *cb_mem;
  /* 用户提供的控制块内存大小，当前适配层保留。 */
  uint32_t                   cb_size;
  /* 用户提供的消息队列存储地址，当前适配层保留。 */
  void                      *mq_mem;
  /* 用户提供的消息队列存储大小，当前适配层保留。 */
  uint32_t                   mq_size;
} osMessageQueueAttr_t;

/*
 * @brief 初始化CMSIS-RTOS v2内核适配层。
 *
 * @retval #osOK 初始化成功。
 * @retval #其他值 初始化失败。
 */
osStatus_t osKernelInitialize (void);

/*
 * @brief 获取CMSIS-RTOS v2内核版本和标识信息。
 *
 * @param version [OUT] 版本信息输出地址。
 * @param id_buf [OUT] 内核标识字符串输出缓冲区。
 * @param id_size [IN] 输出缓冲区大小。
 *
 * @retval #osOK 获取成功。
 * @retval #其他值 获取失败。
 */
osStatus_t osKernelGetInfo (osVersion_t *version, char *id_buf, uint32_t id_size);

/*
 * @brief 获取内核状态。
 *
 * @retval 当前内核状态。
 */
osKernelState_t osKernelGetState (void);

/*
 * @brief 启动CMSIS-RTOS v2内核适配层。
 *
 * @retval #osOK 启动成功。
 */
osStatus_t osKernelStart (void);

/*
 * @brief 锁任务调度。
 *
 * @retval 锁定前的内核锁状态；负值表示失败。
 */
int32_t osKernelLock (void);

/*
 * @brief 解锁任务调度。
 *
 * @retval 解锁前的内核锁状态；负值表示失败。
 */
int32_t osKernelUnlock (void);

/*
 * @brief 恢复任务调度锁状态。
 *
 * @param lock [IN] 需要恢复的锁状态。
 *
 * @retval 恢复后的内核锁状态；负值表示失败。
 */
int32_t osKernelRestoreLock (int32_t lock);

/*
 * @brief 挂起内核调度。
 *
 * @retval 可休眠tick数，当前适配层返回0。
 */
uint32_t osKernelSuspend (void);

/*
 * @brief 恢复内核调度。
 *
 * @param sleep_ticks [IN] 已休眠tick数。
 */
void osKernelResume (uint32_t sleep_ticks);

/*
 * @brief 获取系统tick计数。
 *
 * @retval 当前系统tick计数。
 */
uint64_t osKernelGetTickCount (void);

/*
 * @brief 获取当前系统tick对应的毫秒数。
 *
 * @retval 当前系统时间，单位为毫秒。
 */
uint64_t osKernelGetTick2ms(void);

/*
 * @brief 将毫秒转换为系统tick数。
 *
 * @param ticks [IN] 时间，单位为毫秒。
 *
 * @retval 转换后的系统tick数。
 */
uint64_t osMs2Tick(uint64_t ticks);

/*
 * @brief 获取系统tick频率。
 *
 * @retval 系统tick频率。
 */
uint32_t osKernelGetTickFreq (void);

/*
 * @brief 获取系统定时器计数。
 *
 * @retval 系统定时器计数。
 */
uint32_t osKernelGetSysTimerCount (void);

/*
 * @brief 获取系统定时器频率。
 *
 * @retval 系统定时器频率。
 */
uint32_t osKernelGetSysTimerFreq (void);

/*
 * @brief 创建线程。
 *
 * @param func [IN] 线程入口函数。
 * @param argument [IN] 传递给线程入口函数的参数。
 * @param attr [IN] 线程属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 线程对象句柄。
 */
osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);

/*
 * @brief 获取线程名称。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #NULL 获取失败或无线程名。
 * @retval #非NULL 线程名称字符串地址。
 */
const char *osThreadGetName (osThreadId_t thread_id);

/*
 * @brief 获取当前线程句柄。
 *
 * @retval #NULL 获取失败。
 * @retval #非NULL 当前线程对象句柄。
 */
osThreadId_t osThreadGetId (void);

/*
 * @brief 获取线程状态。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval 线程状态，失败时返回#osThreadError。
 */
osThreadState_t osThreadGetState (osThreadId_t thread_id);

/*
 * @brief 获取线程栈大小。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval 线程栈大小，单位为字节；0表示失败。
 */
uint32_t osThreadGetStackSize (osThreadId_t thread_id);

/*
 * @brief 获取线程栈剩余空间。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval 线程栈剩余空间，单位为字节；0表示失败或不支持。
 */
uint32_t osThreadGetStackSpace (osThreadId_t thread_id);

/*
 * @brief 设置线程优先级。
 *
 * @param thread_id [IN] 线程对象句柄。
 * @param priority [IN] CMSIS-RTOS v2线程优先级。
 *
 * @retval #osOK 设置成功。
 * @retval #其他值 设置失败。
 */
osStatus_t osThreadSetPriority (osThreadId_t thread_id, osPriority_t priority);

/*
 * @brief 获取线程优先级。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval 线程优先级，失败时返回#osPriorityError。
 */
osPriority_t osThreadGetPriority (osThreadId_t thread_id);

/*
 * @brief 主动让出当前线程调度。
 *
 * @retval #osOK 让出成功。
 * @retval #其他值 让出失败。
 */
osStatus_t osThreadYield (void);

/*
 * @brief 挂起指定线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 挂起成功。
 * @retval #其他值 挂起失败。
 */
osStatus_t osThreadSuspend (osThreadId_t thread_id);

/*
 * @brief 恢复指定线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 恢复成功。
 * @retval #其他值 恢复失败。
 */
osStatus_t osThreadResume (osThreadId_t thread_id);

/*
 * @brief 分离线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 分离成功。
 * @retval #其他值 分离失败。
 */
osStatus_t osThreadDetach (osThreadId_t thread_id);

/*
 * @brief 等待线程结束。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 等待成功。
 * @retval #其他值 等待失败。
 */
osStatus_t osThreadJoin (osThreadId_t thread_id);

/*
 * @brief 退出当前线程。
 */
__NO_RETURN void osThreadExit (void);

/*
 * @brief 删除指定线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osThreadTerminate (osThreadId_t thread_id);

/*
 * @brief 获取线程数量。
 *
 * @retval 当前线程数量；0表示失败或不支持。
 */
uint32_t osThreadGetCount (void);

/*
 * @brief 枚举线程句柄。
 *
 * @param thread_array [OUT] 保存线程句柄的数组。
 * @param array_items [IN] 数组元素个数。
 *
 * @retval 写入数组的线程数量；0表示失败或不支持。
 */
uint32_t osThreadEnumerate (osThreadId_t *thread_array, uint32_t array_items);

/*
 * @brief 设置线程标志。
 *
 * @param thread_id [IN] 线程对象句柄。
 * @param flags [IN] 需要设置的标志。
 *
 * @retval 设置后的标志或#osFlagsError系列错误码。
 */
uint32_t osThreadFlagsSet (osThreadId_t thread_id, uint32_t flags);

/*
 * @brief 清除当前线程标志。
 *
 * @param flags [IN] 需要清除的标志。
 *
 * @retval 清除前的标志或#osFlagsError系列错误码。
 */
uint32_t osThreadFlagsClear (uint32_t flags);

/*
 * @brief 获取当前线程标志。
 *
 * @retval 当前线程标志或#osFlagsError系列错误码。
 */
uint32_t osThreadFlagsGet (void);

/*
 * @brief 等待当前线程标志。
 *
 * @param flags [IN] 需要等待的标志。
 * @param options [IN] 等待选项。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval 满足条件的标志或#osFlagsError系列错误码。
 */
uint32_t osThreadFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout);

/*
 * @brief 当前线程延时。
 *
 * @param ticks [IN] 延时时间，单位为tick。
 *
 * @retval #osOK 延时成功。
 * @retval #其他值 延时失败。
 */
osStatus_t osDelay (uint32_t ticks);

/*
 * @brief 当前线程延时到指定tick。
 *
 * @param ticks [IN] 目标tick计数。
 *
 * @retval #osOK 延时成功。
 * @retval #其他值 延时失败。
 */
osStatus_t osDelayUntil (uint64_t ticks);

/*
 * @brief 创建定时器。
 *
 * @param func [IN] 定时器回调函数。
 * @param type [IN] 定时器类型。
 * @param argument [IN] 传递给定时器回调函数的参数。
 * @param attr [IN] 定时器属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 定时器对象句柄。
 */
osTimerId_t osTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr);

/*
 * @brief 获取定时器名称。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 定时器名称字符串地址。
 */
const char *osTimerGetName (osTimerId_t timer_id);

/*
 * @brief 启动定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 * @param ticks [IN] 定时时间，单位为tick。
 *
 * @retval #osOK 启动成功。
 * @retval #其他值 启动失败。
 */
osStatus_t osTimerStart (osTimerId_t timer_id, uint32_t ticks);

/*
 * @brief 停止定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #osOK 停止成功。
 * @retval #其他值 停止失败。
 */
osStatus_t osTimerStop (osTimerId_t timer_id);

/*
 * @brief 查询定时器是否正在运行。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #0 未运行或查询失败。
 * @retval #1 正在运行。
 */
uint32_t osTimerIsRunning (osTimerId_t timer_id);

/*
 * @brief 删除定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osTimerDelete (osTimerId_t timer_id);

/*
 * @brief 创建事件标志对象。
 *
 * @param attr [IN] 事件标志属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 事件标志对象句柄。
 */
osEventFlagsId_t osEventFlagsNew (const osEventFlagsAttr_t *attr);

/*
 * @brief 获取事件标志对象名称。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 事件标志对象名称字符串地址。
 */
const char *osEventFlagsGetName (osEventFlagsId_t ef_id);

/*
 * @brief 设置事件标志。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 * @param flags [IN] 需要设置的标志。
 *
 * @retval 设置后的标志或#osFlagsError系列错误码。
 */
uint32_t osEventFlagsSet (osEventFlagsId_t ef_id, uint32_t flags);

/*
 * @brief 清除事件标志。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 * @param flags [IN] 需要清除的标志。
 *
 * @retval 清除前的标志或#osFlagsError系列错误码。
 */
uint32_t osEventFlagsClear (osEventFlagsId_t ef_id, uint32_t flags);

/*
 * @brief 获取事件标志。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 *
 * @retval 当前事件标志或#osFlagsError系列错误码。
 */
uint32_t osEventFlagsGet (osEventFlagsId_t ef_id);

/*
 * @brief 等待事件标志。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 * @param flags [IN] 需要等待的标志。
 * @param options [IN] 等待选项。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval 满足条件的标志或#osFlagsError系列错误码。
 */
uint32_t osEventFlagsWait (osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);

/*
 * @brief 删除事件标志对象。
 *
 * @param ef_id [IN] 事件标志对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osEventFlagsDelete (osEventFlagsId_t ef_id);

/*
 * @brief 创建互斥锁。
 *
 * @param attr [IN] 互斥锁属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 互斥锁对象句柄。
 */
osMutexId_t osMutexNew (const osMutexAttr_t *attr);

/*
 * @brief 获取互斥锁名称。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 互斥锁名称字符串地址。
 */
const char *osMutexGetName (osMutexId_t mutex_id);

/*
 * @brief 获取互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval #osOK 获取成功。
 * @retval #其他值 获取失败。
 */
osStatus_t osMutexAcquire (osMutexId_t mutex_id, uint32_t timeout);

/*
 * @brief 释放互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus_t osMutexRelease (osMutexId_t mutex_id);

/*
 * @brief 获取互斥锁持有者。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #NULL 获取失败或无持有者。
 * @retval #非NULL 线程对象句柄。
 */
osThreadId_t osMutexGetOwner (osMutexId_t mutex_id);

/*
 * @brief 删除互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osMutexDelete (osMutexId_t mutex_id);

/*
 * @brief 创建信号量。
 *
 * @param max_count [IN] 最大计数值。
 * @param initial_count [IN] 初始计数值。
 * @param attr [IN] 信号量属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 信号量对象句柄。
 */
osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr);

/*
 * @brief 获取信号量名称。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 信号量名称字符串地址。
 */
const char *osSemaphoreGetName (osSemaphoreId_t semaphore_id);

/*
 * @brief 获取信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval #osOK 获取成功。
 * @retval #其他值 获取失败。
 */
osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout);

/*
 * @brief 释放信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id);

/*
 * @brief 获取信号量当前计数。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval 当前计数值；0也可能表示查询失败。
 */
uint32_t osSemaphoreGetCount (osSemaphoreId_t semaphore_id);

/*
 * @brief 删除信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id);

/*
 * @brief 创建内存池。
 *
 * @param block_count [IN] 内存块数量。
 * @param block_size [IN] 单个内存块大小，单位为字节。
 * @param attr [IN] 内存池属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 内存池对象句柄。
 */
osMemoryPoolId_t osMemoryPoolNew (uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr);

/*
 * @brief 获取内存池名称。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 内存池名称字符串地址。
 */
const char *osMemoryPoolGetName (osMemoryPoolId_t mp_id);

/*
 * @brief 从内存池分配内存块。
 *
 * @param mp_id [IN] 内存池对象句柄。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval #NULL 分配失败。
 * @retval #非NULL 内存块地址。
 */
void *osMemoryPoolAlloc (osMemoryPoolId_t mp_id, uint32_t timeout);

/*
 * @brief 释放内存池块。
 *
 * @param mp_id [IN] 内存池对象句柄。
 * @param block [IN] 待释放内存块地址。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus_t osMemoryPoolFree (osMemoryPoolId_t mp_id, void *block);

/*
 * @brief 获取内存池容量。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval 内存池块总数；0表示失败。
 */
uint32_t osMemoryPoolGetCapacity (osMemoryPoolId_t mp_id);

/*
 * @brief 获取内存池块大小。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval 单个内存块大小，单位为字节；0表示失败。
 */
uint32_t osMemoryPoolGetBlockSize (osMemoryPoolId_t mp_id);

/*
 * @brief 获取内存池已使用块数量。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval 已使用块数量；0也可能表示失败。
 */
uint32_t osMemoryPoolGetCount (osMemoryPoolId_t mp_id);

/*
 * @brief 获取内存池空闲块数量。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval 空闲块数量；0也可能表示失败。
 */
uint32_t osMemoryPoolGetSpace (osMemoryPoolId_t mp_id);

/*
 * @brief 删除内存池。
 *
 * @param mp_id [IN] 内存池对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osMemoryPoolDelete (osMemoryPoolId_t mp_id);

/*
 * @brief 创建消息队列。
 *
 * @param msg_count [IN] 消息队列深度。
 * @param msg_size [IN] 单条消息大小，单位为字节。
 * @param attr [IN] 消息队列属性，NULL表示使用默认属性。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 消息队列对象句柄。
 */
osMessageQueueId_t osMessageQueueNew (uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr);

/*
 * @brief 获取消息队列名称。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval #NULL 获取失败或无名称。
 * @retval #非NULL 消息队列名称字符串地址。
 */
const char *osMessageQueueGetName (osMessageQueueId_t mq_id);

/*
 * @brief 发送消息到消息队列。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 * @param msg_ptr [IN] 待发送消息地址。
 * @param msg_prio [IN] 消息优先级，当前适配层保留。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval #osOK 发送成功。
 * @retval #其他值 发送失败。
 */
osStatus_t osMessageQueuePut (osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);

/*
 * @brief 从消息队列接收消息。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 * @param msg_ptr [OUT] 接收消息的缓冲区地址。
 * @param msg_prio [OUT] 消息优先级输出地址，当前适配层保留。
 * @param timeout [IN] 等待超时时间，单位为tick。
 *
 * @retval #osOK 接收成功。
 * @retval #其他值 接收失败。
 */
osStatus_t osMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);

/*
 * @brief 获取消息队列容量。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval 消息队列深度；0表示失败。
 */
uint32_t osMessageQueueGetCapacity (osMessageQueueId_t mq_id);

/*
 * @brief 获取消息大小。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval 单条消息大小，单位为字节；0表示失败。
 */
uint32_t osMessageQueueGetMsgSize (osMessageQueueId_t mq_id);

/*
 * @brief 获取消息队列当前消息数量。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval 当前消息数量；0也可能表示失败。
 */
uint32_t osMessageQueueGetCount (osMessageQueueId_t mq_id);

/*
 * @brief 获取消息队列剩余空间。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval 剩余消息槽数量；0也可能表示失败。
 */
uint32_t osMessageQueueGetSpace (osMessageQueueId_t mq_id);

/*
 * @brief 清空消息队列。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval #osOK 清空成功。
 * @retval #其他值 清空失败。
 */
osStatus_t osMessageQueueReset (osMessageQueueId_t mq_id);

/*
 * @brief 删除消息队列。
 *
 * @param mq_id [IN] 消息队列对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus_t osMessageQueueDelete (osMessageQueueId_t mq_id);

#ifdef  __cplusplus
}
#endif

#endif

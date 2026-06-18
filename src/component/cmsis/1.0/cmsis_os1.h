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
 * Description: CMSIS-RTOS v1 API definitions for UniProton.
 */

#ifndef _CMSIS_OS1_H
#define _CMSIS_OS1_H

/* CMSIS-RTOS v1 API版本号。 */
#define osCMSIS           0x10002

/* UniProton CMSIS-RTOS v1适配层版本号。 */
#define osCMSIS_LOS    0x10000

/* UniProton CMSIS-RTOS v1内核标识字符串。 */
#define osKernelSystemId "LOS V1.0.0"

/* CMSIS-RTOS v1特性开关和能力上限。 */
#define osFeature_MainThread   1
#define osFeature_Pool         1
#define osFeature_MailQ        1
#define osFeature_MessageQ     1
#define osFeature_Signals      8
#define osFeature_Semaphore    30
#define osFeature_Wait         1
#define osFeature_SysTick      1

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* CMSIS-RTOS v1线程优先级。 */
typedef enum  {
  osPriorityIdle          = -3,
  osPriorityLow           = -2,
  osPriorityBelowNormal   = -1,
  osPriorityNormal        =  0,
  osPriorityAboveNormal   = +1,
  osPriorityHigh          = +2,
  osPriorityRealtime      = +3,
  osPriorityError         =  0x84
} osPriority;

/* CMSIS-RTOS v1永久等待超时值。 */
#define osWaitForever     0xFFFFFFFF

/* CMSIS-RTOS v1接口返回状态。 */
typedef enum  {
  osOK                    =     0,
  osEventSignal           =  0x08,
  osEventMessage          =  0x10,
  osEventMail             =  0x20,
  osEventTimeout          =  0x40,
  osErrorParameter        =  0x80,
  osErrorResource         =  0x81,
  osErrorTimeoutResource  =  0xC1,
  osErrorISR              =  0x82,
  osErrorISRRecursive     =  0x83,
  osErrorPriority         =  0x84,
  osErrorNoMemory         =  0x85,
  osErrorValue            =  0x86,
  osErrorOS               =  0xFF,
  os_status_reserved      =  0x7FFFFFFF
} osStatus;

/* CMSIS-RTOS v1定时器类型。 */
typedef enum  {
  osTimerOnce             =     0,
  osTimerPeriodic         =     1,
  osTimerDelay            =     2
} os_timer_type;

/* 线程入口函数类型。 */
typedef void (*os_pthread) (void const *argument);

/* 定时器回调函数类型。 */
typedef void (*os_ptimer) (void const *argument);

/* 线程对象句柄。 */
typedef void *osThreadId;

/* 定时器对象句柄。 */
typedef void *osTimerId;

/* 互斥锁对象句柄。 */
typedef void *osMutexId;

/* 信号量对象句柄。 */
typedef void *osSemaphoreId;

/* 内存池对象句柄。 */
typedef struct os_pool_cb *osPoolId;

/* 消息队列对象句柄。 */
typedef struct os_messageQ_cb *osMessageQId;

/* 邮件队列对象句柄。 */
typedef struct os_mailQ_cb *osMailQId;

/*
 * 线程定义结构体。
 *
 * 用于通过osThreadDef定义线程静态属性。
 */
typedef struct os_thread_def  {
  /* 线程名。 */
  char                       *name;
  /* 线程入口函数。 */
  os_pthread               pthread;
  /* 线程初始优先级。 */
  osPriority             tpriority;
  /* 线程实例数。 */
  uint32_t               instances;
  /* 线程栈大小，单位为字节；0表示使用默认栈大小。 */
  uint32_t               stacksize;
} osThreadDef_t;

/* 定时器定义结构体。 */
typedef struct os_timer_def  {
  /* 定时器回调函数。 */
  os_ptimer                 ptimer;
} osTimerDef_t;

/* 互斥锁定义结构体。 */
typedef struct os_mutex_def  {
  /* 保留字段。 */
  uint32_t                   dummy;
} osMutexDef_t;

/* 信号量定义结构体。 */
typedef struct os_semaphore_def  {
  /* 保留字段。 */
  uint32_t                   dummy;
} osSemaphoreDef_t;

/* 内存池定义结构体。 */
typedef struct os_pool_def  {
  /* 内存池块数量。 */
  uint32_t                 pool_sz;
  /* 单个内存块大小，单位为字节。 */
  uint32_t                 item_sz;
  /* 预留内存池地址；NULL表示由适配层分配。 */
  void                       *pool;
} osPoolDef_t;

/* 消息队列定义结构体。 */
typedef struct os_messageQ_def  {
  /* 消息队列深度。 */
  uint32_t                queue_sz;
  /* 单条消息大小，单位为字节。 */
  uint32_t                 item_sz;
  /* 预留队列存储地址；NULL表示由适配层分配。 */
  void                       *pool;
} osMessageQDef_t;

/* 邮件队列定义结构体。 */
typedef struct os_mailQ_def  {
  /* 邮件队列深度。 */
  uint32_t                queue_sz;
  /* 单封邮件大小，单位为字节。 */
  uint32_t                 item_sz;
  /* 预留邮件队列存储地址；NULL表示由适配层分配。 */
  void                       *pool;
} osMailQDef_t;

/* CMSIS-RTOS v1事件返回结构体。 */
typedef struct  {
  /* 事件状态。 */
  osStatus                 status;
  union  {
    /* 消息值。 */
    uint32_t                    v;
    /* 邮件或用户指针。 */
    void                       *p;
    /* 信号标志。 */
    int32_t               signals;
  } value;
  union  {
    /* 邮件队列句柄。 */
    osMailQId             mail_id;
    /* 消息队列句柄。 */
    osMessageQId       message_id;
  } def;
} osEvent;

/*
 * @brief 初始化CMSIS-RTOS v1内核适配层。
 *
 * @retval #osOK 初始化成功。
 */
osStatus osKernelInitialize (void);

/*
 * @brief 启动CMSIS-RTOS v1内核适配层。
 *
 * @retval #osOK 启动成功。
 */
osStatus osKernelStart (void);

/*
 * @brief 获取内核运行状态。
 *
 * @retval #1 内核处于运行状态。
 */
int32_t osKernelRunning(void);

#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))

/*
 * @brief 获取系统tick计数。
 *
 * @retval 当前系统tick计数。
 */
uint32_t osKernelSysTick (void);

/* CMSIS-RTOS v1系统tick频率。 */
#define osKernelSysTickFrequency 100000000

/* 将微秒转换为系统tick数。 */
#define osKernelSysTickMicroSec(microsec) (((uint64_t)(microsec) * (osKernelSysTickFrequency)) / 1000000)

#endif

/* 定义或引用线程静态对象。 */
#if defined (osObjectsExternal)
#define osThreadDef(name, priority, instances, stacksz)  \
extern const osThreadDef_t os_thread_def_##name
#else
#define osThreadDef(name, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (name), (priority), (instances), (stacksz)  }
#endif

/* 获取线程静态对象地址。 */
#define osThread(name)  \
&os_thread_def_##name

/*
 * @brief 创建线程。
 *
 * @param thread_def [IN] 线程定义结构体。
 * @param argument [IN] 传递给线程入口函数的参数。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 线程对象句柄。
 */
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument);
/*
 * @brief 使用用户栈参数创建线程。
 *
 * @param thread_def [IN] 线程定义结构体。
 * @param stackPointer [IN] 用户栈地址，当前适配层保留。
 * @param stackSize [IN] 用户栈大小，当前适配层保留。
 * @param argument [IN] 传递给线程入口函数的参数。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 线程对象句柄。
 */
osThreadId osUsrThreadCreate (const osThreadDef_t *thread_def, void *stackPointer, uint32_t stackSize, void *argument);

/*
 * @brief 获取当前线程句柄。
 *
 * @retval #NULL 获取失败。
 * @retval #非NULL 当前线程对象句柄。
 */
osThreadId osThreadGetId (void);

/*
 * @brief 删除指定线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osThreadTerminate (osThreadId thread_id);
/*
 * @brief 挂起当前线程。
 *
 * @retval #osOK 挂起成功。
 * @retval #其他值 挂起失败。
 */
osStatus osThreadSelfSuspend (void);
/*
 * @brief 恢复指定线程。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval #osOK 恢复成功。
 * @retval #其他值 恢复失败。
 */
osStatus osThreadResume (osThreadId thread_id);

/*
 * @brief 主动让出当前线程调度。
 *
 * @retval #osOK 让出成功。
 * @retval #其他值 让出失败。
 */
osStatus osThreadYield (void);

/*
 * @brief 设置线程优先级。
 *
 * @param thread_id [IN] 线程对象句柄。
 * @param priority [IN] CMSIS-RTOS v1线程优先级。
 *
 * @retval #osOK 设置成功。
 * @retval #其他值 设置失败。
 */
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);

/*
 * @brief 获取线程优先级。
 *
 * @param thread_id [IN] 线程对象句柄。
 *
 * @retval 线程优先级，失败时返回#osPriorityError。
 */
osPriority osThreadGetPriority (osThreadId thread_id);

/*
 * @brief 当前线程延时。
 *
 * @param millisec [IN] 延时时间，单位为毫秒。
 *
 * @retval #osOK 延时成功。
 * @retval #其他值 延时失败。
 */
osStatus osDelay (uint32_t millisec);

#if (defined (osFeature_Wait)  &&  (osFeature_Wait != 0))

/*
 * @brief 等待任意CMSIS事件。
 *
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval CMSIS-RTOS v1事件结果。
 */
osEvent osWait (uint32_t millisec);

#endif

/* 定义或引用定时器静态对象。 */
#if defined (osObjectsExternal)
#define osTimerDef(name, function)  \
extern const osTimerDef_t os_timer_def_##name
#else
#define osTimerDef(name, function)  \
const osTimerDef_t os_timer_def_##name = \
{ (function) }
#endif

/* 获取定时器静态对象地址。 */
#define osTimer(name) \
&os_timer_def_##name

/*
 * @brief 创建定时器。
 *
 * @param timer_def [IN] 定时器定义结构体。
 * @param type [IN] 定时器类型。
 * @param argument [IN] 传递给定时器回调函数的参数。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 定时器对象句柄。
 */
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument);

/*
 * @brief 启动定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 * @param millisec [IN] 定时时间，单位为毫秒。
 *
 * @retval #osOK 启动成功。
 * @retval #其他值 启动失败。
 */
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec);

/*
 * @brief 停止定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #osOK 停止成功。
 * @retval #其他值 停止失败。
 */
osStatus osTimerStop (osTimerId timer_id);

/*
 * @brief 重启定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 * @param millisec [IN] 定时时间，单位为毫秒。
 * @param strict [IN] 是否使用严格重启语义。
 *
 * @retval #osOK 重启成功。
 * @retval #其他值 重启失败。
 */
osStatus osTimerRestart (osTimerId timer_id, uint32_t millisec, uint8_t strict);

/*
 * @brief 删除定时器。
 *
 * @param timer_id [IN] 定时器对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osTimerDelete (osTimerId timer_id);

/*
 * @brief 设置线程信号标志。
 *
 * @param thread_id [IN] 线程对象句柄。
 * @param signals [IN] 需要设置的信号标志。
 *
 * @retval 非负值表示信号标志结果，负值表示失败。
 */
int32_t osSignalSet (osThreadId thread_id, int32_t signals);

/*
 * @brief 清除线程信号标志。
 *
 * @param thread_id [IN] 线程对象句柄。
 * @param signals [IN] 需要清除的信号标志。
 *
 * @retval 非负值表示清除前信号标志，负值表示失败。
 */
int32_t osSignalClear (osThreadId thread_id, int32_t signals);

/*
 * @brief 等待线程信号标志。
 *
 * @param signals [IN] 需要等待的信号标志。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval CMSIS-RTOS v1事件结果。
 */
osEvent osSignalWait (int32_t signals, uint32_t millisec);

/* 定义或引用互斥锁静态对象。 */
#if defined (osObjectsExternal)
#define osMutexDef(name)  \
extern const osMutexDef_t os_mutex_def_##name
#else
#define osMutexDef(name)  \
const osMutexDef_t os_mutex_def_##name = { 0 }
#endif

/* 获取互斥锁静态对象地址。 */
#define osMutex(name)  \
&os_mutex_def_##name

/*
 * @brief 创建互斥锁。
 *
 * @param mutex_def [IN] 互斥锁定义结构体。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 互斥锁对象句柄。
 */
osMutexId osMutexCreate (const osMutexDef_t *mutex_def);

/*
 * @brief 获取互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval #osOK 获取成功。
 * @retval #其他值 获取失败。
 */
osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec);

/*
 * @brief 释放互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus osMutexRelease (osMutexId mutex_id);

/*
 * @brief 删除互斥锁。
 *
 * @param mutex_id [IN] 互斥锁对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osMutexDelete (osMutexId mutex_id);

#if (defined (osFeature_Semaphore)  &&  (osFeature_Semaphore != 0))

/* 定义或引用信号量静态对象。 */
#if defined (osObjectsExternal)
#define osSemaphoreDef(name)  \
extern const osSemaphoreDef_t os_semaphore_def_##name
#else
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }
#endif

/* 获取信号量静态对象地址。 */
#define osSemaphore(name)  \
&os_semaphore_def_##name

/*
 * @brief 创建计数信号量。
 *
 * @param semaphore_def [IN] 信号量定义结构体。
 * @param count [IN] 初始计数值。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 信号量对象句柄。
 */
osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);

/*
 * @brief 创建二值信号量。
 *
 * @param semaphore_def [IN] 信号量定义结构体。
 * @param count [IN] 初始计数值。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 信号量对象句柄。
 */
osSemaphoreId osBinarySemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count);

/*
 * @brief 获取信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval 非负值表示剩余计数，负值表示失败。
 */
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);

/*
 * @brief 释放信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);

/*
 * @brief 删除信号量。
 *
 * @param semaphore_id [IN] 信号量对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id);

#endif

#if (defined (osFeature_Pool)  &&  (osFeature_Pool != 0))

/* 定义或引用内存池静态对象。 */
#if defined (osObjectsExternal)
#define osPoolDef(name, no, type)   \
extern const osPoolDef_t os_pool_def_##name
#else
#define osPoolDef(name, no, type)   \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), NULL }
#endif

/* 获取内存池静态对象地址。 */
#define osPool(name) \
&os_pool_def_##name

/*
 * @brief 创建内存池。
 *
 * @param pool_def [IN] 内存池定义结构体。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 内存池对象句柄。
 */
osPoolId osPoolCreate (const osPoolDef_t *pool_def);

/*
 * @brief 从内存池分配内存块。
 *
 * @param pool_id [IN] 内存池对象句柄。
 *
 * @retval #NULL 分配失败。
 * @retval #非NULL 内存块地址。
 */
void *osPoolAlloc (osPoolId pool_id);

/*
 * @brief 从内存池分配并清零内存块。
 *
 * @param pool_id [IN] 内存池对象句柄。
 *
 * @retval #NULL 分配失败。
 * @retval #非NULL 内存块地址。
 */
void *osPoolCAlloc (osPoolId pool_id);

/*
 * @brief 释放内存池块。
 *
 * @param pool_id [IN] 内存池对象句柄。
 * @param block [IN] 待释放内存块地址。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus osPoolFree (osPoolId pool_id, void *block);

/*
 * @brief 删除内存池。
 *
 * @param pool_id [IN] 内存池对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osPoolDelete (osPoolId pool_id);

#endif

#if (defined (osFeature_MessageQ)  &&  (osFeature_MessageQ != 0))

/* 定义或引用消息队列静态对象。 */
#if defined (osObjectsExternal)
#define osMessageQDef(name, queue_sz, type)   \
extern const osMessageQDef_t os_messageQ_def_##name
#else
#define osMessageQDef(name, queue_sz, type)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type), NULL }
#endif

/* 获取消息队列静态对象地址。 */
#define osMessageQ(name) \
&os_messageQ_def_##name

/*
 * @brief 创建消息队列。
 *
 * @param queue_def [IN] 消息队列定义结构体。
 * @param thread_id [IN] 关联线程句柄，当前适配层保留。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 消息队列对象句柄。
 */
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id);

/*
 * @brief 向消息队列尾部发送消息。
 *
 * @param queue_id [IN] 消息队列对象句柄。
 * @param info [IN] 消息值。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval #osOK 发送成功。
 * @retval #其他值 发送失败。
 */
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);

/*
 * @brief 向消息队列头部发送消息。
 *
 * @param queue_id [IN] 消息队列对象句柄。
 * @param info [IN] 消息值。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval #osOK 发送成功。
 * @retval #其他值 发送失败。
 */
osStatus osMessagePutHead (const osMessageQId queue_id, uint32_t info, uint32_t millisec);

/*
 * @brief 从消息队列接收消息。
 *
 * @param queue_id [IN] 消息队列对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval CMSIS-RTOS v1事件结果。
 */
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);

/*
 * @brief 删除消息队列。
 *
 * @param queue_id [IN] 消息队列对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osMessageDelete (const osMessageQId queue_id);

#endif

#if (defined (osFeature_MailQ)  &&  (osFeature_MailQ != 0))

/* 定义或引用邮件队列静态对象。 */
#if defined (osObjectsExternal)
#define osMailQDef(name, queue_sz, type) \
extern const osMailQDef_t os_mailQ_def_##name
#else
struct osMailQ {
    uint32_t queue;
    void *pool;
};
#define osMailQDef(name, queue_sz, type) \
struct osMailQ os_mailQ_p_##name = { 0, NULL }; \
const osMailQDef_t os_mailQ_def_##name =  \
{ (queue_sz), sizeof(type), &os_mailQ_p_##name }
#endif

/* 获取邮件队列静态对象地址。 */
#define osMailQ(name)  \
&os_mailQ_def_##name

/*
 * @brief 创建邮件队列。
 *
 * @param queue_def [IN] 邮件队列定义结构体。
 * @param thread_id [IN] 关联线程句柄，当前适配层保留。
 *
 * @retval #NULL 创建失败。
 * @retval #非NULL 邮件队列对象句柄。
 */
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id);

/*
 * @brief 分配邮件内存。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval #NULL 分配失败。
 * @retval #非NULL 邮件内存地址。
 */
void *osMailAlloc (osMailQId queue_id, uint32_t millisec);

/*
 * @brief 分配并清零邮件内存。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval #NULL 分配失败。
 * @retval #非NULL 邮件内存地址。
 */
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec);

/*
 * @brief 向邮件队列尾部发送邮件。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param mail [IN] 邮件内存地址。
 *
 * @retval #osOK 发送成功。
 * @retval #其他值 发送失败。
 */
osStatus osMailPut (osMailQId queue_id, void *mail);

/*
 * @brief 向邮件队列头部发送邮件。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param mail [IN] 邮件内存地址。
 *
 * @retval #osOK 发送成功。
 * @retval #其他值 发送失败。
 */
osStatus osMailPutHead (osMailQId queue_id, void *mail);

/*
 * @brief 从邮件队列接收邮件。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param millisec [IN] 等待超时时间，单位为毫秒。
 *
 * @retval CMSIS-RTOS v1事件结果。
 */
osEvent osMailGet (osMailQId queue_id, uint32_t millisec);

/*
 * @brief 释放邮件内存。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 * @param mail [IN] 邮件内存地址。
 *
 * @retval #osOK 释放成功。
 * @retval #其他值 释放失败。
 */
osStatus osMailFree (osMailQId queue_id, void *mail);

/*
 * @brief 清空邮件队列。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 *
 * @retval #osOK 清空成功。
 * @retval #其他值 清空失败。
 */
osStatus osMailClear (osMailQId queue_id);

/*
 * @brief 删除邮件队列。
 *
 * @param queue_id [IN] 邮件队列对象句柄。
 *
 * @retval #osOK 删除成功。
 * @retval #其他值 删除失败。
 */
osStatus osMailDelete (osMailQId queue_id);

#endif

#ifdef __cplusplus
}
#endif

#endif

# UniProton 接口描述说明书

## 1 简介

本文描述了UniProton对外提供的接口设计，重点描述了接口原型和字段定义等。本文档作为UniProton接口设计的基线文档，能够长期维护与使用。本文档将主要用于指导UniProton和所有使用UniProton接口的子系统的相关开发，也帮助测试人员了解UniProton提供的对外接口设计，支持相关开发和测试人员其完成UniProton及其内部各模块的开发和测试。

### 1.1目的

本文主要用于指导UniProton的开发和测试人员了解UniProton对外提供的接口设计，以支持其完成UniProton各模块的开发和测试。该文档为UniProton接口设计的基线文档，长期维护与使用。

### 1.2全量范围

本文档所列接口用于嵌入式产品。

## 2 总体描述

### 2.1 公共设计约定

#### 2.1.1 处理器平台支持

UniProton支持各类处理器平台，当前仅支持cortex_m4芯片等，定义两个宏来识别一个确定的处理器平台：
     OS_HARDWARE_PLATFORM：代表当前UniProton版本支持的处理器内核，它被赋值为以下枚举值之一：
       OS_CORTEX_M4
    基于各类处理器平台，UniProton向各类用户软件提供的基本统一的接口集合。对于一些具有特殊硬件功能的处理器，UniProton还为其提供相关的应用解决方案，相应地增加一些用户接口。
    本文主要描述基于cortex_m4处理器硬件平台的UniProton接口。
    
    #define OS_HARDWARE_PLATFORM    OS_CORTEX_M4
    #define OS_CPU_TYPE             OS_STM32F407

#### 2.1.2 功能裁剪

UniProton支持各项功能的裁减。其中，对于裁减与否不影响实时运行效率的功能，UniProton通过配置文件prt_config.h里的裁减宏来实现功能裁减，这种方式不要求重新编译UniProton库文件。

UniProton还有部分功能的裁减会影响实时运行效率，对此，UniProton在prt_buildef.h文件中提供裁减宏，改变裁减宏的值需要重新编译UniProton库文件。

#### 2.1.3 基本数据类型

为了屏蔽各种处理器平台在数据类型定义上的差异，UniProton对C语言基本数据类型进行统一定义，供UniProton各功能模块及业务使用。基本数据类型定义为：
```c
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;
typedef signed char S8;
typedef signed short S16;
typedef signed int S32;
typedef signed long long S64;
```

#### 2.1.4 常用宏

UniProton 还提供以下常用宏定义。

```c
prt_typedef.h
#ifndef OS_SEC_ALW_INLINE
#define OS_SEC_ALW_INLINE
#endif
#ifndef INLINE
#define INLINE static __inline __attribute__((always_inline))
#endif
#ifndef OS_EMBED_ASM
#define OS_EMBED_ASM __asm__ __volatile__
#endif
#define ALIGN(addr, boundary) (((uintptr_t)(addr) + (boundary) - 1) & ~((uintptr_t)(boundary) - 1))
#define TRUNCATE(addr, size)  ((addr) & ~((uintptr_t)(size) - 1))
#endif
#define YES 1
#endif
#define NO 0
#ifndef FALSE
#define FALSE ((bool)0)
#endif
#ifndef TRUE
#define TRUE ((bool)1)
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif
#define OS_ERROR   (U32)(-1)
#define OS_INVALID (-1)
#ifndef OS_OK
#define OS_OK 0
#endif
#ifndef LIKELY
#define LIKELY(x) __builtin_expect(!!(x), 1)
#endif
#ifndef UNLIKELY
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif
```
## 3 接口定义
### 3.1 prt接口
#### 3.1.1 时钟模块
##### 3.1.1.1 获取当前的64位time stamp计数(即系统运行的cycles)
    【接口原型】
    U64 PRT_ClkGetCycleCount64(void)
    【功能描述】
    获取当前的64位time stamp计数(即系统运行的cycles)。
    【返回值】
    [0,0xFFFFFFFFFFFFFFFF] 系统当前的cycle数
    【参数说明】
    无。
    【注意事项】
    获取的是64bit cycles 数据。

##### 3.1.1.2 转换cycle为毫秒
    【接口原型】
    U64 PRT_ClkCycle2Ms(U64 cycle)
    【功能描述】
    转换cycle为毫秒。
    【返回值】
    [0,0xFFFFFFFFFFFFFFFF] 转换后的毫秒数。
    【参数说明】
    cycle [IN]  类型#U64，cycle数。
    【注意事项】
    None

##### 3.1.1.3 转换cycle为微秒
    【接口原型】
    U64 PRT_ClkCycle2Us(U64 cycle)
    【功能描述】
    转换cycle为微秒。
    【返回值】
    [0,0xFFFFFFFFFFFFFFFF] 转换后的微秒数。
    【参数说明】
    cycle [IN]  类型#U64，cycle数。
    【注意事项】
    None

##### 3.1.1.4 延迟时间(单位微秒)
    【接口原型】
    void PRT_ClkDelayUs(U32 delay)
    【功能描述】
    延迟时间(单位微秒)。
    【返回值】
    无
    【参数说明】
    delay [IN]  类型#U32，延迟微秒数。
    【注意事项】
    None

##### 3.1.1.5 延迟时间(单位毫秒)
    【接口原型】
    void PRT_ClkDelayMs(U32 delay)
    【功能描述】
    延迟时间(单位毫秒)。
    @param delay [IN]  类型#U32，延迟毫秒数。
    @retval 无
    【返回值】
    无
    【参数说明】
    delay [IN]  类型#U32，延迟微秒数。
    【注意事项】
    None

#### 3.1.2 硬中断模块
##### 3.1.2.1 激活指定核号内的硬中断
    【接口原型】
    U32 PRT_HwiTrigger(U32 dstCore, HwiHandle hwiNum)
    【功能描述】
    激活指定核号内的软件可触发的硬中断
    【返回值】
    #OS_OK  0x00000000，硬中断激活成功。
    #其它值，激活失败。
    【参数说明】
    dstCore [IN]  类型#U32，目标核号。目前只支持指定为本核。
    hwiNum  [IN]  类型#HwiHandle，硬中断号，只支持软件可触发的中断号。
    【注意事项】
    None

##### 3.1.2.2 清空中断请求位
    【接口原型】
    void PRT_HwiClearAllPending(void)
    【功能描述】
    清除所有的中断请求位。即放弃当前已触发中断的的响应。
    【返回值】
    无。
    【参数说明】
    无。
    【注意事项】
    None

##### 3.1.2.3 清除硬中断的Pending位
    【接口原型】
    U32 PRT_HwiClearPendingBit(HwiHandle hwiNum)
    【功能描述】
    显式清除硬中断或事件的请求位，因为有的硬件响应中断后不会自动清Pending位。
    【返回值】
    #OS_OK  0x00000000，硬中断请求位清除成功。
    #其它值，清除失败。
    【参数说明】
    hwiNum [IN]  类型#HwiHandle，硬中断号。
    【注意事项】
    None

##### 3.1.2.4 屏蔽指定的硬中断
    【接口原型】
    U32 PRT_HwiDisable(HwiHandle hwiNum)
    【功能描述】
    禁止核响应指定硬中断的请求。
    【返回值】
    #OS_OK  0x00000000，硬中断去使能成功。
    #其它值，硬中断去使能失败。
    【参数说明】
    hwiNum [IN]  类型#HwiHandle，依据不同的芯片，硬中断号或中断向量号，见注意事项。
    【注意事项】
    None

##### 3.1.2.5 使能指定的硬中断
    【接口原型】
    U32 PRT_HwiEnable(HwiHandle hwiNum)
    【功能描述】
    允许核响应指定硬中断的请求。
    【返回值】
    #OS_OK  0x00000000，硬中断使能成功。
    #其它值，硬中断使能失败。
    【参数说明】
    hwiNum [IN]  类型#HwiHandle，依据不同的芯片，硬中断号或中断向量号，见注意事项。
    【注意事项】
    对于不同芯片，此返回值代表的意义有所差异，差异细节见下面返回值说明

##### 3.1.2.6 添加硬中断进入钩子
    【接口原型】
    U32 PRT_HwiAddEntryHook(HwiEntryHook hook)
    【功能描述】
    添加硬中断进入钩子。该钩子函数在进入硬中断ISR前被调用。
    【返回值】
    #OS_OK  0x00000000，硬中断使能成功。
    #其它值，硬中断使能失败。
    【参数说明】
    hook [IN]  类型#HwiEntryHook，中断进入钩子函数。
    【注意事项】
    不同钩子函数间执行的先后顺序，不应当存在依赖关系。
    不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。
    最大支持钩子数需静态配置

##### 3.1.2.7 删除硬中断进入钩子
    【接口原型】
    U32 PRT_HwiDelEntryHook(HwiEntryHook hook)
    【功能描述】
    删除硬中断进入钩子。该钩子函数将停止在进入硬中断ISR前的调用。
    【返回值】
    #OS_OK  0x00000000，硬中断使能成功。
    #其它值，硬中断使能失败。
    【参数说明】
    hook [IN]  类型#HwiEntryHook，中断进入钩子函数。
    【注意事项】
    不同钩子函数间执行的先后顺序，不应当存在依赖关系。
    不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。
    最大支持钩子数需静态配置

##### 3.1.2.8 添加硬中断退出钩子
    【接口原型】
    U32 PRT_HwiAddExitHook(HwiExitHook hook)
    【功能描述】
    添加硬中断退出钩子。该钩子函数在退出硬中断ISR后被调用。
    【返回值】
    #OS_OK  0x00000000，硬中断使能成功。
    #其它值，硬中断使能失败。
    【参数说明】
    hook [IN]  类型#HwiExitHook，中断退出钩子函数。
    【注意事项】
    不同钩子函数间执行的先后顺序，不应当存在依赖关系。
    不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。
    最大支持钩子数需静态配置

##### 3.1.2.9 删除硬中断退出钩子
    【接口原型】
    U32 PRT_HwiDelExitHook(HwiExitHook hook)
    【功能描述】
    删除硬中断退出钩子。该钩子函数将停止在退出硬中断ISR后的调用。
    【返回值】
    #OS_OK  0x00000000，硬中断使能成功。
    #其它值，硬中断使能失败。
    【参数说明】
    hook [IN]  类型#HwiExitHook，中断退出钩子函数。
    【注意事项】
    不同钩子函数间执行的先后顺序，不应当存在依赖关系。
    不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。
    最大支持钩子数需静态配置

##### 3.1.2.10 开中断
    【接口原型】
    uintptr_t PRT_HwiUnLock(void)
    【功能描述】
    开启全局可屏蔽中断。
    【返回值】
    开启全局中断前的中断状态值。
    【参数说明】
    无。
    【注意事项】
    None

##### 3.1.2.11 关中断
    【接口原型】
    uintptr_t PRT_HwiLock(void)
    【功能描述】
    关闭全局可屏蔽中断。
    【返回值】
    关闭全局中断前的中断状态值。
    【参数说明】
    无。
    【注意事项】
    None

##### 3.1.2.12 恢复中断状态接口
    【接口原型】
    void PRT_HwiRestore(uintptr_t intSave)
    【功能描述】
    恢复原中断状态寄存器。
    【返回值】
    无
    【参数说明】
    intSave [IN]  类型#uintptr_t，关全局中断PRT_IntLock和开全局中断PRT_IntUnLock的返回值。
    【注意事项】
    该接口必须和关闭全局中断或者是开启全局中断接口成对使用，以关全局中断或者开全局中断操作的返回值为入参
    以保证中断可以恢复到关全局中断或者开全局中断操作前的状态

##### 3.1.2.13 设置硬中断属性接口
    【接口原型】
    U32 PRT_HwiSetAttr(HwiHandle hwiNum, HwiPrior hwiPrio, HwiMode mode)
    【功能描述】
    在创建硬中断前，必须要配置好硬中断的优先级和模式，包括独立型（#OS_HWI_MODE_ENGROSS）和
    组合型（#OS_HWI_MODE_COMBINE）两种配置模式。
    【返回值】
    #OS_OK  0x00000000，硬中断属性设置成功。
    #其它值，属性设置失败。
    【参数说明】
    hwiNum  [IN]  类型#HwiHandle，硬中断号。
    hwiPrio [IN]  类型#HwiPrior，硬中断优先级。
    mode    [IN]  类型#HwiMode，设置的中断模式，为独立型(#OS_HWI_MODE_ENGROSS)或者组合型(#OS_HWI_MODE_COMBINE)。
    【注意事项】
    OS已经占用的不能被使用

##### 3.1.2.14 创建硬中断函数
    【接口原型】
    U32 PRT_HwiCreate(HwiHandle hwiNum, HwiProcFunc handler, HwiArg arg)
    【功能描述】
    注册硬中断的处理函数。
    【返回值】
    #OS_OK  0x00000000，硬中断创建成功。
    #其它值，创建失败。
    【参数说明】
    hwiNum  [IN]  类型#HwiHandle，硬中断号。
    handler [IN]  类型#HwiProcFunc，硬中断触发时的处理函数。
    arg     [IN]  类型#HwiArg，调用硬中断处理函数时传递的参数。
    【注意事项】
    在调用该函数之前，请先确保已经设置了中断属性。
    硬中断创建成功后，并不使能相应向量的中断，需要显式调用#PRT_HwiEnable单独使能。

##### 3.1.2.15 删除硬中断函数
    【接口原型】
    U32 PRT_HwiDelete(HwiHandle hwiNum)
    【功能描述】
    屏蔽相应硬中断或事件，取消硬中断处理函数的注册。
    【返回值】
    #OS_OK  0x00000000，硬中断删除成功。
    #其它值，删除失败。
    【参数说明】
    hwiNum [IN]  类型#HwiHandle，硬中断号。
    【注意事项】
    不能删除OS占用的中断号。

#### 3.1.3 事件模块
##### 3.1.3.1 读事件
    【接口原型】
    U32 PRT_EventRead(U32 eventMask, U32 flags, U32 timeOut, U32 *events)
    【功能描述】
    读取当前任务的指定掩码为eventMask的事件，可以一次性读取多个事件。事件读取成功后，被读取的事件将被清除。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    eventMask [IN]  类型#U32，设置要读取的事件掩码，每个bit位对应一个事件，1表示要读取。该入参不能为0。
    flags     [IN]  类型#U32，读取事件所采取的策略, 为（OS_EVENT_ANY，OS_EVENT_ALL）中一个和（OS_EVENT_WAIT，
    OS_EVENT_NOWAIT）中的一个标识或的结果。#OS_EVENT_ALL表示期望接收eventMask中的所有事件，
    #OS_EVENT_ANY表示等待eventMask中的任何一个事件。#OS_EVENT_WAIT表示若期望事件没有发生，等待接收，
    #OS_EVENT_NOWAIT表示若期望事件没有发生，将不会等待接收。
    timeOut   [IN]  类型#U32，等待超时时间，单位为tick，取值(0~0xFFFFFFFF]。当flags标志为OS_EVENT_WAIT，
    这个参数才有效。若值为#OS_EVENT_WAIT_FOREVER，则表示永久等待。
    events    [OUT] 类型#U32 *，用于保存接收到的事件的指针。如果不需要输出，可以填写NULL。
    【注意事项】
    读取模式可以选择读取任意事件和读取所有事件。
    等待模式可以选择等待和不等待，可等待指定时间、永久等待。
    当读取事件没有发生且为等待模式时，会发生任务调度，锁任务状态除外。
    不能在IDLE任务中读事件，需要用户自行保证。
    不能在系统初始化之前调用读事件接口。

##### 3.1.3.2 写事件
    【接口原型】
    U32 PRT_EventWrite(U32 taskId, U32 events)
    【功能描述】
    写任务ID为taskId的指定事件，可以一次性写多个事件，可以在UniProton接管的中断中调用。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    taskId [IN]  类型#U32，任务ID，表示要对某个任务进行写事件操作。
    events [IN]  类型#U32，事件号，每个bit对应一个事件。
    【注意事项】
    若指定任务正在等待读取写入的事件，则会激活指定任务，并发生任务调度。
    不能向IDLE任务写事件，需要用户自行保证。

#### 3.1.4 异常模块
##### 3.1.4.1 用户注册异常处理钩子
    【接口原型】
    U32 PRT_ExcRegHook(ExcProcFunc hook)
    【功能描述】
    注册异常处理钩子。
    【返回值】
    #OS_OK  0x00000000，注册失败。
    #其它值，注册失败。
    【参数说明】
    hook [IN]  类型#ExcProcFunc，钩子函数。
    【注意事项】
    当多次注册该钩子时，最后一次注册的钩子生效。
    注册的钩子函数不能为空，即一旦注册钩子函数，不能通过注册空函数将其取消。

#### 3.1.5 背景任务模块
##### 3.1.5.1 注册IDLE循环中调用的钩子
    【接口原型】
    U32 PRT_IdleAddHook(IdleHook hook)
    【功能描述】
    注册在IDLE任务中调用的钩子函数。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    hook [OUT] 类型#IdleHook，IDLE钩子函数，该参数不能为空。
    【注意事项】
    钩子中不能调用引起任务阻塞或切换的函数。

##### 3.1.5.2 删除IDLE循环中调用的钩子
    【接口原型】
    U32 PRT_IdleDelHook(IdleHook hook)
    【功能描述】
    删除在IDLE任务中调用的钩子函数。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    hook [OUT] 类型#IdleHook，IDLE钩子函数，该参数不能为空。
    【注意事项】
    None

##### 3.1.5.3 注册IDLE循环进入前调用的钩子
    【接口原型】
    U32 PRT_IdleAddPrefixHook(IdleHook hook)
    【功能描述】
    为本核注册IDLE循环进入前调用的钩子，该钩子只会被调用一次。
    【返回值】
    #OS_OK                          0x00000000，操作成功。
    【参数说明】
    hook [IN]  类型#IdleHook，钩子函数。
    【注意事项】
    注册的钩子只在进入IDLE循环前执行一次。
    若任务未裁剪，则作用的是任务IDLE循环。
    IDLE任务钩子中使用矢量寄存器需要在前置钩子中调用#PRT_TaskCpSaveCfg接口设置矢量操作上下文保护区。

#### 3.1.6 队列模块
##### 3.1.6.1 创建队列
    【接口原型】
    U32 PRT_QueueCreate(U16 nodeNum, U16 maxNodeSize, U32 *queueId)
    【功能描述】
    创建一个队列，创建时可以设定队列长度和队列结点大小。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    nodeNum     [IN]  类型#U16，队列节点个数，不能为0。
    maxNodeSize [IN]  类型#U16，每个队列结点的大小。
    queueId     [OUT] 类型#U32 *，存储队列ID，ID从1开始。
    【注意事项】
    每个队列节点的大小的单位是BYTE。
    每个队列节点的长度自动做2字节对齐。
    每个队列节点的长度不能大于0xFFFA。

##### 3.1.6.2 读队列
    【接口原型】
    U32 PRT_QueueRead(U32 queueId, void *bufferAddr, U32 *len, U32 timeOut)
    【功能描述】
    读取指定队列中的数据。将读取到的数据存入bufferAddr地址，bufferAddr地址和读取数据大小由用户传入。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    queueId    [IN]  类型#U32，队列ID。
    bufferAddr [OUT] 类型#void *，读取存放队列中数据的起始地址。
    len        [I/O] 类型#U32 *，传入BUF的大小，输出实际消息的大小。
    timeOut    [IN]  类型#U32，超时时间。
    【注意事项】
    队列读取才采用FIFO模式，即先入先出，读取队列中最早写入的数据(相对于队列节点是先后顺序)。
    如果bufferSize大于队列中实际消息的大小，则只返回实际大小的数据，否则只读取bufferSize大小的数据。
    bufferSize大小的单位是BYTE。
    阻塞模式不能在idle钩子使用，需用户保证。
    在osStart之前不能调用该接口，需用户保证。

##### 3.1.6.3 写队列
    【接口原型】
    U32 PRT_QueueWrite(U32 queueId, void *bufferAddr, U32 bufferSize, U32 timeOut, U32 prio)
    【功能描述】
    向指定队列写数据。将bufferAddr地址中bufferSize大小的数据写入到队列中。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    queueId    [IN]  类型#U32，队列ID。
    bufferAddr [IN]  类型#void *，写到队列中数据的起始地址。
    bufferSize [IN]  类型#U32，写到队列中数据的大小。
    timeOut    [IN]  类型#U32，超时时间。
    prio       [IN]  类型#U32，优先级, 取值OS_QUEUE_NORMAL或OS_QUEUE_URGENT。
    【注意事项】
    需保证bufferSize大小小于或等于队列结点大小。
    bufferSize大小的单位是BYTE。                
    阻塞模式不能在idle钩子使用，需用户保证。      
    在osStart之前不能调用该接口，需用户保证。     

##### 3.1.6.4 删除队列
    【接口原型】
    U32 PRT_QueueDelete(U32 queueId)
    【功能描述】
    删除一个消息队列。删除后队列资源被回收。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    queueId [IN]  类型#U32，队列ID。
    【注意事项】
    不能删除未创建的队列。
    删除同步队列时，必须确保任务阻塞于该队列，且无被激活后还没及时操作队列的任务，否则删除队列失败。

##### 3.1.6.5 获取队列的历史最大使用长度
    【接口原型】
    U32 PRT_QueueGetUsedPeak(U32 queueId, U32 *queueUsedNum)
    【功能描述】
    获取从队列创建到删除前的历史最大使用长度。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    queueId      [IN]  类型#U32，队列ID
    queueUsedNum [OUT] 类型#U32 *，队列节点使用峰值
    【注意事项】
    峰值在队列删除前，不会被清零。

##### 3.1.6.6 获取指定源PID的待处理消息个数
    【接口原型】
    U32 PRT_QueueGetNodeNum(U32 queueId, U32 taskPid, U32 *queueNum)
    【功能描述】
    从指定队列中，获取指定源PID的待处理消息个数。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    queueId  [IN]  类型#U32，队列ID。
    taskPid  [IN]  类型#U32，线程PID。
    queueNum [OUT] 类型#U32 *，待处理的消息个数。
    【注意事项】
    PID为OS_QUEUE_PID_ALL时，表示获取所有待处理的消息个数 
    PID的合法性不做判断，不合法的PID获取的消息个数为0     

#### 3.1.7 信号量模块
##### 3.1.7.1 创建一个计数型信号量
    【接口原型】
    U32 PRT_SemCreate(U32 count, SemHandle *semHandle)
    【功能描述】
    根据用户指定的计数值，创建一个计数型信号量，设定初始计数器数值，唤醒方式为FIFO。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    count     [IN]  类型#U32，计数器初始值，取值范围为[0, 0xFFFFFFFE]。
    semHandle [OUT] 类型#SemHandle *，输出信号量句柄。
    【注意事项】
    创建是否成功会受到"核内信号量裁剪开关"和"最大支持信号量"配置项的限制。

##### 3.1.7.2 删除一个信号量
    【接口原型】
    U32 PRT_SemDelete(SemHandle semHandle)
    【功能描述】
    删除用户传入的信号量句柄指定的信号量，如果有任务阻塞于该信号量，则删除失败。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    【注意事项】
    None

##### 3.1.7.3 获取信号量计数器数值
    【接口原型】
    U32 PRT_SemGetCount(SemHandle semHandle, U32 *semCnt)
    【功能描述】
    根据用户输入信号量句柄和计数值，获取信号量计数器数值。
    【返回值】
    #OS_OK  0x00000000，获取信号量计数器值成功。
    #其它值，获取失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    semCnt    [OUT] 类型#U32 *，保存信号量计数值指针。
    【注意事项】
    None

##### 3.1.7.4 等待一个信号量
    【接口原型】
    U32 PRT_SemPend(SemHandle semHandle, U32 timeout)
    【功能描述】
    等待用户传入信号量句柄指定的信号量，若其计数器值大于0，则直接减1返回成功。否则任务阻塞，
    等待其他线程发布该信号量，等待超时时间可设定。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    timeout   [IN]  类型#U32，等待时间限制，单位为tick，取值范围为[0, 0xffffffff]。#OS_NO_WAIT或0表示不等待，
    #OS_WAIT_FOREVER或0xffffffff表示永久等待。
    【注意事项】
    在osStart之前不能调用该接口。
    等待时间可以选择零等待、等待特定时间、永久等待。
    该接口只能被任务调用。
    在锁任务情况下，用户要PEND信号量，要保证当前有可用信号量资源。

##### 3.1.7.5 发布指定的信号量
    【接口原型】
    U32 PRT_SemPost(SemHandle semHandle)
    【功能描述】
    发布指定的信号量，若没有任务等待该信号量，则直接将计数器加1返回。
    否则根据唤醒方式唤醒相应的阻塞任务，FIFO方式唤醒最早阻塞的任务，优先级方式唤醒阻塞在此信号量的最高优先级任务。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    【注意事项】
    在osStart之前不能调用该接口。
    在未锁任务的情况下，如果唤醒的任务优先级高于当前任务，则会立刻发生任务切换。
    发生任务切换时，如果支持优先级唤醒方式，且创建信号量时指定唤醒方式为优先级，

##### 3.1.7.6 获取阻塞在指定核内信号量上的任务PID清单
    【接口原型】
    U32 PRT_SemGetPendList(SemHandle semHandle, U32 *tskCnt, U32 *pidBuf, U32 bufLen)
    【功能描述】
    根据用户指定的核内信号量句柄，获取阻塞在指定核内信号量上的任务PID清单。
    若有任务阻塞于该核内信号量，则返回阻塞于该核内信号量的任务数目，以及相应任务的PID。
    若没有任务阻塞于该核内信号量，则返回阻塞于该核内信号量的任务数目为0。
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    tskCnt    [OUT] 类型#U32 *，阻塞于该核内信号量的任务数。
    pidBuf    [OUT] 类型#U32 *，由用户指定的内存区域首地址，用于存储阻塞于指定核内信号量的任务PID。
    bufLen    [IN]  类型#U32，用户指定的内存区域的长度（单位：字节）。
    【注意事项】
    用户应保证存储任务PID清单的内存空间足够大，建议将bufLen配置为(#OS_TSK_MAX_SUPPORT_NUM + 1)

##### 3.1.7.7 获取信号量详细信息:信号量当前计数值，信号量持有者(最后一次pend成功的线程ID)，信号量唤醒方式，信号量类型
    【接口原型】
    U32 PRT_SemGetInfo(SemHandle semHandle, struct SemInfo *semInfo)
    【功能描述】
    根据用户输入信号量句柄获取信号量详细信息。
    【返回值】
    #OS_OK  0x00000000，获取信号量计数器值成功。
    #其它值，获取失败。
    【参数说明】
    semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
    semInfo   [OUT] 类型#struct SemInfo *，信号量详细信息:count--信号量计数，owner--信号量占用者，
    mode--信号量唤醒方式，type--信号量类型。
    【注意事项】
    None

#### 3.1.8 系统模块
##### 3.1.8.1 复位单板
    【接口原型】
    void PRT_SysReboot(void)
    【功能描述】
    复位单板，程序重新加载执行。
    【返回值】
    无
    【参数说明】
    无。
    【注意事项】
    并非直接复位单板，而是关中断，等待看门狗复位。
    用户可以通过配置项OS_SYS_REBOOT_HOOK挂接复位函数。

##### 3.1.8.2 获取当前核ID
    【接口原型】
    U32 PRT_SysGetCoreId(void)
    【功能描述】
    获取当前核ID。
    【返回值】
    物理核ID。
    【参数说明】
    无。
    【注意事项】
    获取的核ID为硬件寄存器中的ID号。

##### 3.1.8.3 业务给OS传递RND值，用作后续相关功能模块的保护
    【接口原型】
    U32 PRT_SysSetRndNum(enum SysRndNumType type, U32 rndNum)
    【功能描述】
    None
    【返回值】
    #OS_OK  0x00000000，操作成功。
    #其它值，操作失败。
    【参数说明】
    type   [IN]  类型#enum SysRndNumType，设置的目标RND值的类型。
    rndNum [IN]  类型#U32，传递的RND值。
    【注意事项】
    栈保护随机值设置必须在PRT_HardBootInit中调用。

##### 3.1.8.4 获取OS的版本号
    【接口原型】
    char *PRT_SysGetOsVersion(void)
    【功能描述】
    获取OS的版本号。版本编号格式为UniProton xx.xx VERSIONID(XXX)。
    【返回值】
    指向OS版本号的字符串指针。
    【参数说明】
    无。
    【注意事项】
    None

#### 3.1.9 任务模块
##### 3.1.9.1 创建任务，但不激活任务
    【接口原型】
    U32 PRT_TaskCreate(TskHandle *taskPid, struct TskInitParam *initParam)
    【功能描述】
    创建一个任务。该任务不加入就绪队列，只处于挂起状态，用户需要激活该任务需要通过调用PRT_TaskResume函数将其激活。
    【返回值】
    #OS_OK  0x00000000，任务创建成功。
    #其它值，创建失败。
    【参数说明】
    taskPid   [OUT] 类型#TskHandle *，保存任务PID。
    initParam [IN]  类型#struct TskInitParam *，任务创建参数，
    其结构体中的成员参数stackAddr传入时必须进行初始化，若不采用用户配置的独立任务栈进行栈空间分配，
    该成员必须初始化为0。
    【注意事项】
    任务创建时，会对之前自删除任务的任务控制块和任务栈进行回收，用户独立配置的任务栈除外。
    任务名的最大长度为16字节，含结束符。
    同一核内任务名不允许重复。
    若指定的任务栈大小为0，则使用配置项#OS_TSK_DEFAULT_STACK_SIZE指定的默认的任务栈大小。
    任务栈的大小必须按16字节大小对齐。确定任务栈大小的原则是，够用就行：多了浪费，少了任务栈溢出。
    具体多少取决于需要消耗多少的栈内存，视情况而定：函数调用层次越深，栈内存开销越大，
    局部数组越大，局部变量越多，栈内存开销也越大。
    用户配置的任务栈首地址需16字节对齐。
    用户配置的任务栈空间需由用户保证其合法性，即对可cache空间的地址用户需要保证其任务栈首地址及栈大小cache line对齐，系统不做对齐处理，并在使用后需由用户进行释放。
    任务创建时，任务创建参数中的任务栈大小配置建议不要使用最小任务栈大小OS_TSK_MIN_STACK_SIZE，
    该项只是包含任务上下文预留的栈空间，任务调度时额外的任务开销需要由用户自行保证足够的任务栈空间配置。

##### 3.1.9.2 恢复任务
    【接口原型】
    U32 PRT_TaskResume(TskHandle taskPid)
    【功能描述】
    恢复挂起的任务。
    【返回值】
    #OS_OK  0x00000000，恢复任务成功。
    #其它值，恢复任务失败。
    【参数说明】
    taskPid [IN]  类型#TskHandle，任务PID。
    【注意事项】
    在osStart之前不能调用该接口。
    若任务仍处于延时、阻塞态，则只是取消挂起态，并不加入就绪队列。

##### 3.1.9.3 挂起任务
    【接口原型】
    U32 PRT_TaskSuspend(TskHandle taskPid)
    【功能描述】
    挂起指定的任务，任务将从就绪队列中被删除。
    【返回值】
    #OS_OK  0x00000000，挂起任务成功。
    #其它值，挂起任务失败。
    【参数说明】
    taskPid [IN]  类型#TskHandle，任务PID。
    【注意事项】
    在osStart之前不能调用该接口。
    若为当前任务且已锁任务，则不能被挂起。
    IDLE任务不能被挂起。

##### 3.1.9.4 删除任务
    【接口原型】
    U32 PRT_TaskDelete(TskHandle taskPid)
    【功能描述】
    删除指定的任务，释放任务栈和任务控制块资源。
    【返回值】
    #OS_OK  0x00000000，删除任务成功。
    #其它值，删除任务失败。
    【参数说明】
    taskPid [IN]  类型#TskHandle，任务PID。
    【注意事项】
    若为自删除，则任务控制块和任务栈在下一次创建任务时才回收。
    对于任务自删除，处理该任务相关的信号量和接收到的消息会强制删除。
    任务自删除时，删除钩子不允许进行pend信号量、挂起等操作。

##### 3.1.9.5 延迟正在运行的任务
    【接口原型】
    U32 PRT_TaskDelay(U32 tick)
    【功能描述】
    延迟当前运行任务的执行。任务延时等待指定的Tick数后，重新参与调度。
    【返回值】
    #OS_OK  0x00000000，任务延时成功。
    #其它值，延时任务失败。
    【参数说明】
    tick [IN]  类型#U32，延迟的Tick数。
    【注意事项】
    在osStart之前不能调用该接口。
    硬中断处理中，或已锁任务，则延时操作失败。
    若传入参数0，且未锁任务调度，则顺取同优先级队列中的下一个任务执行。如没有同级的就绪任务，
    则不发生任务调度，继续执行原任务。

##### 3.1.9.6 锁任务调度
    【接口原型】
    void PRT_TaskLock(void)
    【功能描述】
    锁任务调度。若任务调度被锁，则不发生任务切换。
    【返回值】
    无
    【参数说明】
    无。
    【注意事项】
    只是锁任务调度，并不关中断，因此任务仍可被中断打断。
    执行此函数则锁计数值加1，解锁则锁计数值减1。因此，必须与#PRT_TaskUnlock配对使用。

##### 3.1.9.7 解锁任务调度
    【接口原型】
    void PRT_TaskUnlock(void)
    【功能描述】
    任务锁计数值减1。若嵌套加锁，则只有锁计数值减为0才真正的解锁了任务调度。
    【返回值】
    无
    【参数说明】
    无。
    【注意事项】
    在osStart之前不能调用该接口。
    执行此函数则锁计数值加1，解锁则锁计数值减1。因此，必须与#PRT_TaskLock配对使用。

##### 3.1.9.8 获取当前任务PID
    【接口原型】
    U32 PRT_TaskSelf(TskHandle *taskPid)
    【功能描述】
    获取处于运行态的任务PID。
    【返回值】
    #OS_OK  0x00000000，获取成功。
    #其它值，获取失败。
    【参数说明】
    taskPid [OUT] 类型#TskHandle *，保存任务PID。
    【注意事项】
    硬中断处理中，也可获取当前任务PID，即被中断打断的任务。
    在任务切换钩子处理中调用时，获取的是切入任务的ID。

##### 3.1.9.9 获取任务状态
    【接口原型】
    TskStatus PRT_TaskGetStatus(TskHandle taskPid)
    【功能描述】
    获取指定任务的状态。
    【返回值】
    #(TskStatus)OS_INVALID    返回失败。
    #任务状态 返回成功。
    【参数说明】
    taskPid [IN]  类型#TskHandle，任务PID。
    【注意事项】
    None

##### 3.1.9.10 获取任务信息
    【接口原型】
    U32 PRT_TaskGetInfo(TskHandle taskPid, struct TskInfo *taskInfo)
    【功能描述】
    获取指定任务的信息。
    【返回值】
    #OS_OK  0x00000000，获取成功。
    #其它值，获取失败。
    【参数说明】
    taskPid  [IN]  类型#TskHandle，任务PID。
    taskInfo [OUT] 类型#struct TskInfo *，保存任务信息。
    【注意事项】
    若获取当前任务的信息，则只在硬中断、异常的处理中才有意义，
    由于任务切换时，上下文信息也保存在任务栈中，因此任务信息中的SP是保存上下文之后的实际的SP值。

##### 3.1.9.11 获取优先级
    【接口原型】
    U32 PRT_TaskGetPriority(TskHandle taskPid, TskPrior *taskPrio)
    【功能描述】
    获取指定任务的优先级。
    【返回值】
    #OS_OK  0x00000000，获取成功。
    #其它值，获取失败。
    【参数说明】
    taskPid  [IN]  类型#TskHandle，任务PID。
    taskPrio [OUT] 类型#TskPrior *，保存任务优先级指针。
    【注意事项】
    None

##### 3.1.9.12 设置优先级
    【接口原型】
    U32 PRT_TaskSetPriority(TskHandle taskPid, TskPrior taskPrio)
    【功能描述】
    设置指定任务的优先级。
    【返回值】
    #OS_OK  0x00000000，设置成功。
    #其它值，设置失败。
    【参数说明】
    taskPid  [IN]  类型#TskHandle，任务PID。
    taskPrio [IN]  类型#TskPrior，任务优先级。
    【注意事项】
    在osStart之前不能调用该接口。
    若设置的优先级高于当前运行的任务，则可能引发任务调度。
    若调整当前运行任务的优先级，同样可能引发任务调度。
    若任务发生优先级继承，或者任务阻塞在互斥信号量或优先级唤醒模式的信号量上，不可以设置任务的优先级。

##### 3.1.9.13 查询本核指定任务正在PEND的信号量
    【接口原型】
    U32 PRT_TaskGetPendSem(TskHandle taskId, U16 *semId, U16 *pendState)
    【功能描述】
    根据任务状态和任务控制块，判断任务是否在PEND信号量，以及PEND的信号量ID。
    【返回值】
    #OS_OK  0x00000000，查询成功。
    #其它值，查询失败。
    【参数说明】
    taskId    [IN]  类型#TskHandle，任务PID。
    semId     [OUT] 类型#U16 *，任务PEND的信号量ID或者#OS_INVALID。
    pendState [OUT] 类型#U16 *，任务的PEND状态：0，#OS_TSK_FSEM_PEND，#OS_TSK_PEND, #OS_TSK_MCSEM_PEND。
    【注意事项】
    用户应先判断PEND状态，状态为0表明任务没有被信号量阻塞。

##### 3.1.9.14 查询任务名
    【接口原型】
    U32 PRT_TaskGetName(TskHandle taskId, char **name)
    【功能描述】
    根据任务PID，查询任务名。
    【返回值】
    #OS_OK  0x00000000，查询成功。
    #其它值，查询失败。
    【参数说明】
    taskId  [IN]  类型#TskHandle，任务ID。
    name [OUT] 类型#char **，保存任务名字符串的首地址。
    【注意事项】
    在osStart之前不能调用该接口。
    不能查询ID不合法的任务名。
    若查询没有创建的任务名，查询失败。

##### 3.1.9.15 注册任务切换钩子
    【接口原型】
    U32 PRT_TaskAddSwitchHook(TskSwitchHook hook)
    【功能描述】
    注册任务切换钩子函数。钩子函数在切入新任务前被调用。
    【返回值】
    #OS_OK  0x00000000，查询成功。
    #其它值，查询失败。
    【参数说明】
    hook [IN]  类型#TskSwitchHook，任务切换钩子函数。
    【注意事项】
    不同钩子函数间执行的先后顺序，不应当存在依赖关系。
    不应在任务切换钩子里进行可能引起任务调度的处理，如：任务延时、P信号量等。

##### 3.1.9.16 取消任务切换钩子
    【接口原型】
    U32 PRT_TaskDelSwitchHook(TskSwitchHook hook)
    【功能描述】
    取消指定的任务切换钩子。钩子函数在切入新任务前被调用。
    【返回值】
    #OS_OK  0x00000000，查询成功。
    #其它值，查询失败。
    【参数说明】
    hook [IN]  类型#TskSwitchHook，任务切换钩子函数。
    【注意事项】
    None

#### 3.1.10 Tick中断
##### 3.1.10.1 获取每秒钟对应的Tick数
    【接口原型】
    U64 PRT_TickGetCount(void)
    【功能描述】
    获取当前的tick计数。
    【返回值】
    [0,0xFFFFFFFFFFFFFFFF] 当前的tick计数。
    【参数说明】
    无。
    【注意事项】
    None

#### 3.1.11 定时器模块
##### 3.1.11.1 创建定时器
    【接口原型】
    U32 PRT_TimerCreate(struct TimerCreatePara *createPara, TimerHandle *tmrHandle)
    【功能描述】
    创建一个属性为createPara的定时器，返回定时器逻辑ID tmrHandle。
    【返回值】
    #OS_OK  0x00000000，定时器创建成功。
    #其它值，创建失败。
    【参数说明】
    createPara [IN]  类型#struct TimerCreatePara *，定时器创建参数
    tmrHandle  [OUT] 类型#TimerHandle *，定时器句柄
    【注意事项】
    如果用户打开Tick开关则可创建硬件定时器个数少一个。
    中断处理函数handler的第一个参数是创建的定时器的逻辑编号。
    定时器创建成功后并不立即开始计数，需显式调用#PRT_TimerStart或者#PRT_TimerRestart启动。
    对于周期定时模式的定时器，建议用户不要把定时间隔设置的过低，避免一直触发定时器的处理函数。
    struct TimerCreatePara参数里面的interval元素表示定时器周期，软件定时器单位是ms，

##### 3.1.11.2 删除定时器
    【接口原型】
    U32 PRT_TimerDelete(U32 mid, TimerHandle tmrHandle)
    【功能描述】
    释放一个定时器逻辑ID为tmrHandle的定时器资源。
    【返回值】
    #OS_OK  0x00000000，定时器删除成功。
    #其它值，删除失败。
    【参数说明】
    mid       [IN]  类型#U32，模块号，当前未使用，忽略
    tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过PRT_TimerCreate接口获取
    【注意事项】
    硬件定时器删除后将停止计数。
    删除处于超时状态下的软件定时器时，OS采用延时删除的方式。详细说明请见手册注意事项。

##### 3.1.11.3 启动定时器
    【接口原型】
    U32 PRT_TimerStart(U32 mid, TimerHandle tmrHandle)
    【功能描述】
    将定时器逻辑ID为tmrHandle的定时器由创建状态变成启动状态。
    【返回值】
    #OS_OK  0x00000000，定时器启动成功。
    #其它值，启动失败。
    【参数说明】
    mid       [IN]  类型#U32，模块号，当前未使用，忽略
    tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
    【注意事项】
    创建后初次启动定时器时，从设置的值开始计数；如果重复启动或者启动后停止然后再启动，
    对于全局硬件定时器，无论是第一次启动还是重复启动，下一次启动都从初始值开始计时。
    对于单次触发模式，触发一次后，可以调用启动接口再次启动该定时器，时间间隔为设置的时间间隔。
    启动处于超时状态下的软件定时器时，OS采用延时启动的方式。详细说明请见手册注意事项。

##### 3.1.11.4 停止定时器
    【接口原型】
    U32 PRT_TimerStop(U32 mid, TimerHandle tmrHandle)
    【功能描述】
    停止定时器逻辑ID为tmrHandle的定时器计数，使定时器由计时状态变成创建状态。
    【返回值】
    #OS_OK  0x00000000，定时器停止成功。
    #其它值，停止失败。
    【参数说明】
    mid       [IN]  类型#U32，模块号，当前未使用，忽略
    tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
    【注意事项】
    定时器停止后，下一次启动将重新从停止点的计数值开始计数。
    不能停止未启动的定时器。
    停止处于超时状态下的软件定时器时，OS采用延时停止的方式。详细说明请见手册注意事项。
    核内硬件定时器在停止过程中如果发生超时，则剩余时间为0，但不会响应定时器处理函数。

##### 3.1.11.5 重启定时器
    【接口原型】
    U32 PRT_TimerRestart(U32 mid, TimerHandle tmrHandle)
    【功能描述】
    重启定时器逻辑ID为tmrHandle的定时器计数，对于停止过的定时器，相当于恢复到刚创建时的定时时长开始计时。
    【返回值】
    #OS_OK  0x00000000，定时器重启成功。
    #其它值，重启失败。
    【参数说明】
    mid       [IN]  类型#U32，模块号，当前未使用，忽略
    tmrHandle [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
    【注意事项】
    重启处于超时状态下的软件定时器时，OS采用延时重启的方式。详细说明请见手册注意事项。

##### 3.1.11.6 定时器剩余超时时间查询
    【接口原型】
    U32 PRT_TimerQuery(U32 mid, TimerHandle tmrHandle, U32 *expireTime)
    【功能描述】
    输入定时器句柄tmrHandle，获得对应定时器超时剩余时间expireTime。
    【返回值】
    #OS_OK  0x00000000，定时器剩余超时时间查询成功。
    #其它值，查询失败。
    【参数说明】
    mid        [IN]  类型#U32，模块号，当前未使用，忽略
    tmrHandle  [IN]  类型#TimerHandle，定时器句柄，通过#PRT_TimerCreate接口获取
    expireTime [OUT] 类型#U32 *，定时器的剩余的超时时间，共享和私有硬件定时器单位us，软件定时器单位ms
    【注意事项】
    软件定时器单位毫秒，核内和全局硬件定时器单位微秒。
    由于OS内部软件定时器采用Tick作为计时单位，硬件定时器采用Cycle作为计时单位，

#### 3.1.12 内存基本功能
##### 3.1.12.1 向已创建的指定分区申请内存
    【接口原型】
    void *PRT_MemAlloc(U32 mid, U8 ptNo, uintptr_t size)
    【功能描述】
    <li>在分区号为ptNo的分区中，申请大小为size的内存。</li>
    【返回值】
    #NULL  0，申请失败。
    #!NULL 内存首地址值。
    【参数说明】
    mid  [IN]  类型#U32，申请的模块号。
    ptNo [IN]  类型#U8，分区号，范围[0,#OS_MEM_MAX_PT_NUM+2)，当前未使用，选择默认分区。
    size [IN]  类型#uintptr_t，申请的大小。
    【注意事项】
    申请内存时的分区号，根据实际创建的分区号来使用。
    调用函数后，注意判断返回的地址是否为空以避免后续访问空指针。
    对于FSC内存算法，申请到的内存首地址是按4字节对齐的 
    如果内存申请失败，返回值为NULL，而导致失败的原因将记录在错误处理空间中。

##### 3.1.12.2 释放申请的内存
    【接口原型】
    U32 PRT_MemFree(U32 mid, void *addr)
    【功能描述】
    该接口根据内存块的地址addr，找到该内存块所属的内存分区，再根据分区号和用户使用的地址addr释放该内存。
    【返回值】
    #OS_OK  0x00000000，内存释放成功。
    #其它值，释放失败。
    【参数说明】
    mid  [IN]  类型#U32，要释放的模块号。
    addr [IN]  类型#void *，释放的地址。
    【注意事项】
    如果返回值不是#OS_OK，则内存不会被释放。
    被破坏的内存不能被释放。
    对于入参addr，OS已做基本校验，无法校验所有非法地址，其合法性由业务保证。

#### 3.1.13 CPU占用率模块
##### 3.1.13.1 获取当前cpu占用率
    【接口原型】
    U32 PRT_CpupNow(void)
    【功能描述】
    通过本接口获取当前cpu占用率。
    【返回值】
    #0xffffffff      获取失败，CPUP裁剪开关未打开，或未初始化，或者在osStart之前调用。
    #[0,10000]       返回当前cpu占用率。
    【参数说明】
    无。
    【注意事项】
    该接口必须在CPUP模块裁剪开关打开，并在osStart之后才能调用此接口，否则返回0xffffffff。
    精度为万分之一。
    为了减小CPUP统计对线程调度的性能影响，OS采用了基于IDLE计数的统计算法，

##### 3.1.13.2 获取指定个数的线程的CPU占用率
    【接口原型】
    U32 PRT_CpupThread(U32 inNum, struct CpupThread *cpup, U32 *outNum)
    【功能描述】
    根据用户输入的线程个数，获取指定个数的线程CPU占用率。
    【返回值】
    #OS_OK  0x00000000，获取成功。
    #其它值，获取失败。
    【参数说明】
    inNum  [IN]  类型#U32，输入的线程个数。
    cpup   [OUT] 类型#struct CpupThread *，缓冲区，输出参数，用于填写输出个数线程的CPUP信息。
    outNum [OUT] 类型#U32 *，保存输出的实际线程个数指针。
    【注意事项】
    当且仅当CPUP模式配置为线程级时，该接口有效。
    当配置项中的采样周期值等于0时，线程级CPUP采样周期为两次调用该接口或者PRT_CpupNow之间
    输出的实际线程个数不大于系统中实际的线程个数(任务个数和一个中断线程)。
    若输入的线程个数为1，则仅输出中断线程(除任务线程以外的线程统称)的CPUP信息。
    若在一个采样周期内有任务被删除，则统计的任务线程和中断线程CPUP总和小于10000。

##### 3.1.13.3 设置CPU占用率告警阈值
    【接口原型】
    U32 PRT_CpupSetWarnValue(U32 warn, U32 resume)
    【功能描述】
    根据用户配置的CPU占用率告警阈值warn和告警恢复阈值resume，设置告警和恢复阈值。
    【返回值】
    #OS_OK  0x00000000，阈值设定成功。
    #其它值，阈值设定失败。
    【参数说明】
    warn   [IN]  类型#U32，CPUP告警阈值。
    resume [IN]  类型#U32，CPUP恢复阈值。
    【注意事项】
    OsStart之前不能调用此接口。

##### 3.1.13.4 查询CPUP告警阈值和告警恢复阈值
    【接口原型】
    U32 PRT_CpupGetWarnValue(U32 *warn, U32 *resume)
    【功能描述】
    根据用户配置的告警阈值指针warn和告警恢复阈值指针resume，查询告警阈值和告警恢复阈值
    【返回值】
    #OS_OK  0x00000000，获取成功。
    #其它值，获取失败。
    【参数说明】
    warn   [OUT] 类型#U32 *，CPUP告警阈值。
    resume [OUT] 类型#U32 *，CPUP恢复阈值。
    【注意事项】
    OsStart之前不能调用此接口。

##### 3.1.13.5 注册CPUP告警回调函数
    【接口原型】
    U32 PRT_CpupRegWarnHook(CpupHookFunc hook)
    【功能描述】
    根据用户配置的回调函数hook，注册CPUP告警回调函数
    【返回值】
    #OS_OK  0x00000000，注册成功。
    #其它值，注册失败。
    【参数说明】
    hook [IN]  类型#CpupHookFunc，CPU告警回调函数。
    【注意事项】
    不允许重复或覆盖注册钩子。
    hook为NULL时，表示删除该钩子。

#### 3.1.14 错误处理
##### 3.1.14.1 处理错误
    【接口原型】
    U32 PRT_ErrHandle(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para)
    【功能描述】
    用户或者OS内部使用该函数回调#PRT_ErrRegHook中注册的钩子函数。另外，OS内部使用此接口时，还会记录相关错误码。
    【返回值】
    #OS_OK  0x00000000，处理错误成功。
    【参数说明】
    fileName [IN]  类型#const char *，错误发生的文件名，可以用__FILE__作为输入。
    lineNo   [IN]  类型#U32，错误发生所在的行号，可以用__LINE__作为输入。
    errorNo  [IN]  类型#U32，用户输入的错误码。
    paraLen  [IN]  类型#U32，入参para的长度。
    para     [OUT] 类型#void *，记录用户对于错误的描述或其他(地址)。
    【注意事项】
    系统不会做入参检测，用户PRT_ErrHandle会全部当做钩子入参输入。
    用户调用PRT_ErrHandle接口时，只会回调用户注册钩子函数，不会进行错误码记录

##### 3.1.14.2 注册用户错误处理的钩子函数
    【接口原型】
    U32 PRT_ErrRegHook(ErrHandleFunc hook)
    【功能描述】
    注册hook作为用户钩子函数，在调用PRT_ErrHandle接口进行错误处理时对该钩子进行调用。
    【返回值】
    #OS_OK  0x00000000，注册成功。
    #其它值，注册失败。
    【参数说明】
    hook [IN]  类型#ErrHandleFunc，用户钩子函数的宏定义。
    【注意事项】
    若入参hook为NULL,则为取消钩子。
    不允许重复或覆盖注册。
    用户定义的钩子函数必须符合#PRT_ERRORHANDLE_FUNC定义的形式，而且只能定义一个钩子函数。
    用户调用PRT_ErrRegHook注册回调钩子函数时，钩子函数里面不能有调用PRT_ErrHandle
    用户调用PRT_ErrRegHook注册回调钩子函数时，钩子函数里面如有单次上报的错误信息（只有第一次调用会执行）

#### 3.1.15 日志模块
##### 3.1.15.1 日志初始化函数
    【接口原型】
    U32 PRT_LogInit(uintptr_t memBase)
    【功能描述】
    将日志内容写入日志环形缓存
    【返回值】
    #OS_OK  0x00000000，日志初始化成功。
    #其它值，日志初始化失败。
    【参数说明】
    memBase [IN]  类型#uintptr_t，日志环形缓存基地址。
    【注意事项】
    该接口只应该调用一次。
    日志模块会使用从基地址开始的17MB内存，注意预留内存。

##### 3.1.15.2 日志初始化函数
    【接口原型】
    bool PRT_IsLogInit(void)
    【功能描述】
    日志是否完成初始化，从核可以使用该函数确认主核是否已完成日志初始化。
    【返回值】
    #true，日志已初始化。
    #false，日志尚未初始化。
    【参数说明】
    无
    【注意事项】
    无

##### 3.1.15.3 日志输出函数
    【接口原型】
    U32 PRT_Log(enum OsLogLevel level, enum OsLogFacility facility, const char *str, size_t strLen)
    【功能描述】
    将日志内容写入日志环形缓存
    【返回值】
    #OS_OK  0x00000000，日志输出成功。
    #其它值，日志输出失败。
    【参数说明】
    level    [IN]  类型#enum OsLogLevel，日志级别，定义同linux syslog level，取值范围[0,7]。
    facility [IN]  类型#enum OsLogFacility，日志来源模块，定义同linux syslog facility，取值范围[16,23]。
    str      [IN]  类型#const char *，日志字符串，取值范围为非空。
    strLen   [IN]  类型#size_t，不包括结束符的字符串长度。
    【注意事项】
    调用该接口前，需要先调用PRT_LogInit，初始化日志环形缓存的基地址。

##### 3.1.15.4 日志格式化输出函数
    【接口原型】
    U32 PRT_LogFormat(enum OsLogLevel level, enum OsLogFacility facility, const char *fmt, ...)
    【功能描述】
    将日志字符串格式化输出，与其他日志内容一起写入日志环形缓存
    【返回值】
    #OS_OK  0x00000000，日志输出成功。
    #其它值，日志输出失败。
    【参数说明】
    level    [IN]  类型#enum OsLogLevel，日志级别，定义同linux syslog level，取值范围[0,7]。
    facility [IN]  类型#enum OsLogFacility，日志来源模块，定义同linux syslog facility，取值范围[16,23]。
    fmt      [IN]  类型#const char *，格式化的控制字符串，取值范围为非空。
    ...      [IN]  可选参数。
    【注意事项】
    调用该接口前，需要先调用PRT_LogInit，初始化日志环形缓存的基地址
    支持vsnprintf所支持的字符串格式化类型。

##### 3.1.15.5 日志功能关闭函数
    【接口原型】
    void PRT_LogOff(void)
    【功能描述】
    关闭日志功能，调用该接口后，PRT_Log与PRT_LogFormat将会失效。
    【返回值】
    无
    【参数说明】
    无
    【注意事项】
    日志功能默认开启

##### 3.1.15.6 日志功能开启函数
    【接口原型】
    void PRT_LogOn(void)
    【功能描述】
    开启日志功能，调用该接口后，PRT_Log与PRT_LogFormat将正常工作。
    【返回值】
    无
    【参数说明】
    无
    【注意事项】
    日志功能默认开启

##### 3.1.15.7 日志过滤函数
    【接口原型】
    U32 PRT_LogSetFilter(enum OsLogLevel level)
    【功能描述】
    设置日志过滤级别，调用该接口后，PRT_Log与PRT_LogFormat输出日志时级别等于或低于过滤级别的日志将被过滤。
    例如, 设定过滤级别为OS_LOG_NONE, 没有日志会被过滤。
    设定过滤级别为OS_LOG_NOTICE, 则OS_LOG_NOTICE, OS_LOG_INFO, OS_LOG_DEBUG级别的日志会被过滤。
    【返回值】
    #OS_OK  0x00000000，设置成功。
    #其它值，设置失败。
    【参数说明】
    level    [IN]  类型#enum OsLogLevel，日志过滤级别，取值范围[0,8]。
    【注意事项】
    无

##### 3.1.15.8 日志根据来源过滤函数
    【接口原型】
    extern U32 PRT_LogSetFilterByFacility(enum OsLogFacility facility, enum OsLogLevel level)
    【功能描述】
    对指定的日志来源模块设置日志过滤级别，调用该接口后，PRT_Log与PRT_LogFormat输出指定模块的日志时级别等于或低于过滤级别的日志将被过滤。
    【返回值】
    #OS_OK  0x00000000，设置成功。
    #其它值，设置失败。
    【参数说明】
    facility [IN]  类型#enum OsLogFacility，日志来源模块，取值范围[16,23]。
    level    [IN]  类型#enum OsLogLevel，日志过滤级别，取值范围[0,8]。
    【注意事项】
    PRT_LogSetFilter相当于对所有日志来源模块设定相同的日志过滤级别，PRT_LogSetFilter与PRT_LogSetFilterByFacility对日志来源模块的过滤设置会互相覆盖。

### 3.2 config接口
    【接口原型】
    OS_SYS_CLOCK
    【功能描述】
    芯片主频 
    【接口原型】
    OS_HWI_MAX_NUM_CONFIG
    【功能描述】
    硬中断最大支持个数 
    【接口原型】
    OS_INCLUDE_TICK
    【功能描述】
    Tick中断模块裁剪开关 
    【接口原型】
    OS_TICK_PER_SECOND
    【功能描述】
    Tick中断时间间隔，tick处理时间不能超过1/OS_TICK_PER_SECOND(s) 
    【接口原型】
    OS_INCLUDE_TICK_SWTMER
    【功能描述】
    基于TICK的软件定时器裁剪开关 
    【接口原型】
    OS_TICK_SWITIMER_MAX_NUM
    【功能描述】
    基于TICK的软件定时器最大个数 
    【接口原型】
    OS_INCLUDE_TASK
    【功能描述】
    任务模块裁剪开关 
    【接口原型】
    OS_TSK_MAX_SUPPORT_NUM
    【功能描述】
    最大支持的任务数,最大共支持254个 
    【接口原型】
    OS_TSK_DEFAULT_STACK_SIZE
    【功能描述】
    缺省的任务栈大小 
    【接口原型】
    OS_TSK_IDLE_STACK_SIZE
    【功能描述】
    IDLE任务栈的大小 
    【接口原型】
    OS_TSK_STACK_MAGIC_WORD
    【功能描述】
    任务栈初始化魔术字，默认是0xCA，只支持配置一个字节 
    【接口原型】
    OS_INCLUDE_CPUP
    【功能描述】
    CPU占用率模块裁剪开关 
    【接口原型】
    OS_CPUP_SAMPLE_INTERVAL
    【功能描述】
    采样时间间隔(单位tick)，若其值大于0，则作为采样周期，否则两次调用PRT_CpupNow或PRT_CpupThread间隔作为周期 
    【接口原型】
    OS_CONFIG_CPUP_WARN
    【功能描述】
    CPU占用率告警动态配置项 
    【接口原型】
    OS_CPUP_SHORT_WARN
    【功能描述】
    CPU占用率告警阈值(精度为万分比) 
    【接口原型】
    OS_CPUP_SHORT_RESUME
    【功能描述】
    CPU占用率告警恢复阈值(精度为万分比) 
    【接口原型】
    OS_MEM_MAX_PT_NUM
    【功能描述】
    用户可以创建的最大分区数，取值范围[0,253] 
    【接口原型】
    OS_MEM_FSC_PT_ADDR
    【功能描述】
    私有FSC内存分区起始地址 
    【接口原型】
    OS_MEM_FSC_PT_SIZE
    【功能描述】
    私有FSC内存分区大小 
    【接口原型】
    OS_INCLUDE_SEM
    【功能描述】
    信号量模块裁剪开关 
    【接口原型】
    OS_SEM_MAX_SUPPORT_NUM
    【功能描述】
    最大支持的信号量数 
    【接口原型】
    OS_INCLUDE_QUEUE
    【功能描述】
    队列模块裁剪开关 
    【接口原型】
    OS_QUEUE_MAX_SUPPORT_NUM
    【功能描述】
    最大支持的队列数,范围(0,0xFFFF] 
    【接口原型】
    OS_HOOK_HWI_ENTRY_NUM
    【功能描述】
    硬中断进入钩子最大支持个数, 范围[0, 255] 
    【接口原型】
    OS_HOOK_HWI_EXIT_NUM
    【功能描述】
    硬中断退出钩子最大支持个数, 范围[0, 255] 
    【接口原型】
    OS_HOOK_TSK_SWITCH_NUM
    【功能描述】
    任务切换钩子最大支持个数, 范围[0, 255] 
    【接口原型】
    OS_HOOK_IDLE_NUM
    【功能描述】
    IDLE钩子最大支持个数, 范围[0, 255] 

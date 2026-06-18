# UniProton CMSIS-RTOS v2适配设计文档

## 1 整体方案

CMSIS-RTOS v2适配层在UniProton上提供`cmsis_os2.h`中的v2接口。版本选择由`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`控制：当`CMSIS_OS_VER=2`时，头文件选择`src/component/cmsis/2.0/cmsis_os2.h`，适配实现只编译`src/component/cmsis/2.0/cmsis_uniproton2.c`中的v2代码。

sd3403平台使用`demos/sd3403/build/build_app.sh cmsis`构建CMSIS验证镜像。用户只需把`cmsis_os.h`中的`CMSIS_OS_VER`改为`2`，组件和测试套都会使用v2。

目录结构：

```text
src/component/cmsis/
|-- cmsis_os.h
|-- cmsis_uniproton.c
|-- cmsis_uniproton.h
`-- 2.0/
    |-- cmsis_os2.h
    `-- cmsis_uniproton2.c

testsuites/cmsis/
|-- common/
|   `-- cmsis_test_log.h
`-- v2/
    |-- cmsis_v2_basic_test.c
    |-- cmsis_sem_e2e_test.c
    |-- cmsis_mutex_e2e_test.c
    |-- cmsis_msgq_e2e_test.c
    |-- cmsis_event_flags_e2e_test.c
    |-- cmsis_thread_flags_e2e_test.c
    |-- cmsis_timer_e2e_test.c
    |-- cmsis_priority_e2e_test.c
    |-- cmsis_suspend_resume_e2e_test.c
    `-- cmsis_kernel_lock_e2e_test.c
```

## 2 编译控制

组件开关：

```text
CONFIG_OS_SUPPORT_CMSIS=y
```

版本控制只使用`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`：

```c
#define CMSIS_OS_VER 2
```

组件CMake不传递版本宏，只读取`cmsis_os.h`中的`CMSIS_OS_VER=2`并编译`src/component/cmsis/2.0/cmsis_uniproton2.c`。测试套读取同一个头文件并只编译`testsuites/cmsis/v2`目录下的文件。

## 3 支持接口

v2适配层覆盖以下CMSIS-RTOS v2接口类别：

| 类别 | 接口范围 |
| --- | --- |
| Kernel | `osKernelInitialize`、`osKernelGetInfo`、`osKernelGetState`、`osKernelStart`、lock/unlock/restore、tick和system timer查询 |
| Thread | `osThreadNew`、name/id/state/stack/priority/count/enumerate查询、yield、suspend/resume、detach/join/exit/terminate |
| Thread Flags | `osThreadFlagsSet`、`osThreadFlagsClear`、`osThreadFlagsGet`、`osThreadFlagsWait` |
| Event Flags | `osEventFlagsNew`、get name、set/clear/get/wait/delete |
| Delay/Timer | `osDelay`、`osDelayUntil`、`osTimerNew`、get name、start/stop/is running/delete |
| Mutex | `osMutexNew`、get name、acquire/release/get owner/delete |
| Semaphore | `osSemaphoreNew`、get name、acquire/release/get count/delete |
| Memory Pool | `osMemoryPoolNew`、get name、alloc/free/capacity/block size/count/space/delete |
| Message Queue | `osMessageQueueNew`、get name、put/get/capacity/msg size/count/space/reset/delete |

## 4 关键实现

线程ID通过任务ID加一映射，避免任务ID 0被误判为NULL。复杂对象使用包装结构保存CMSIS语义需要的额外信息：

| 对象 | 包装内容 |
| --- | --- |
| `CmsisQueue` | UniProton队列ID、消息大小、消息数量 |
| `CmsisSemaphore` | UniProton信号量句柄、最大计数、名称 |
| `CmsisMutex` | mutex semaphore句柄、名称 |
| `CmsisTimer` | 软件定时器句柄、回调、参数、类型、周期 |
| `CmsisEventFlags` | flags、内部唤醒队列、名称 |
| `CmsisMemoryPool` | block大小、容量、已使用数量 |

v2优先级和UniProton优先级方向相反，适配层通过`CmsisPrioToPrt()`和`CmsisPrioFromPrt()`双向转换。`osThreadGetPriority()`返回CMSIS优先级。

Semaphore使用包装对象保存`max_count`，用于实现`osSemaphoreRelease()`超过最大计数时返回`osErrorResource`。

Event Flags内部维护flags并使用长度为1的UniProton queue唤醒阻塞等待线程。Memory Pool当前基于UniProton通用内存逐块分配，不支持等待空闲block。

## 5 测试设计

v2测试位于`testsuites/cmsis/v2`，统一入口为`cmsis_v2_basic_test.c`中的`cmsis_test()`。测试逻辑只调用CMSIS接口；`cmsis_test_log.h`仅作为sd3403串口日志后端封装`PRT_Printf`。

基础API测试覆盖：

| 测试项 | 覆盖内容 |
| --- | --- |
| `kernel` | kernel info/state/lock/unlock、tick和timer频率 |
| `thread` | 创建、名称、栈信息、优先级、删除 |
| `threadFlags` | set、WaitAny、WaitAll |
| `semaphore` | count、非阻塞acquire、timeout、release、overflow、delete |
| `mutex` | acquire、release、delete |
| `mempool` | capacity、block size、alloc/free、count/space、delete |
| `eventFlags` | set、wait、get、clear、delete |
| `queue` | put/get、capacity/msg size/count/space、reset、delete |
| `delay` | delay和delay until边界 |
| `negativePaths` | NULL、非法参数、非法优先级等路径 |

端到端测试每个场景一个文件：

| 文件 | 场景 |
| --- | --- |
| `cmsis_sem_e2e_test.c` | 空信号量阻塞等待，release后唤醒waiter |
| `cmsis_mutex_e2e_test.c` | 两线程竞争mutex，验证owner、非阻塞失败和阻塞唤醒 |
| `cmsis_msgq_e2e_test.c` | producer/consumer通过message queue同步并校验消息顺序 |
| `cmsis_event_flags_e2e_test.c` | event flags跨线程WaitAny/WaitAll同步 |
| `cmsis_thread_flags_e2e_test.c` | thread flags跨线程同步和清除语义 |
| `cmsis_timer_e2e_test.c` | timer回调唤醒等待线程 |
| `cmsis_priority_e2e_test.c` | 验证CMSIS优先级映射和调度顺序 |
| `cmsis_suspend_resume_e2e_test.c` | suspend/resume后线程停止和继续运行 |
| `cmsis_kernel_lock_e2e_test.c` | kernel lock期间调度受控，unlock后线程继续运行 |

## 6 sd3403验证

构建命令：

```bash
sg docker -c "docker exec uniproton_cmsis_verify bash -c 'cd /home/uniproton/UniProton/demos/sd3403/build && sh build_app.sh cmsis'"
```

生成物：

```text
demos/sd3403/build/cmsis.elf
```

板端运行时，`/etc/mica/sd3403.conf`中的`ClientPath`应设置为：

```text
ClientPath=/etc/mica/cmsis.elf
```

预期串口日志前缀为`[cmsis-v2]`，最终通过标志为：

```text
[cmsis-v2][PASS] tick=<tick>
```

## 7 当前限制

| 限制 | 说明 |
| --- | --- |
| Memory Pool等待 | `osMemoryPoolAlloc`不支持等待空闲block，容量耗尽时直接返回`NULL` |
| Message优先级 | `osMessageQueuePut`当前忽略`msg_prio` |
| 名称接口 | `osTimerGetName`和`osMessageQueueGetName`当前返回基础值 |
| Tickless | `osKernelSuspend`和`osKernelResume`为基础占位实现 |

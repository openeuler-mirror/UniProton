# UniProton CMSIS-RTOS v1适配设计文档

## 1 整体方案

CMSIS-RTOS v1适配层在UniProton上提供`cmsis_os.h`中的v1接口。版本选择由`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`控制：当`CMSIS_OS_VER=1`时，头文件选择`src/component/cmsis/1.0/cmsis_os1.h`，适配实现只编译`src/component/cmsis/1.0/cmsis_uniproton1.c`中的v1代码。

sd3403平台使用`demos/sd3403/build/build_app.sh cmsis`构建CMSIS验证镜像。用户只需把`cmsis_os.h`中的`CMSIS_OS_VER`改为`1`，组件和测试套都会使用v1。

目录结构：

```text
src/component/cmsis/
|-- cmsis_os.h
|-- cmsis_uniproton.c
|-- cmsis_uniproton.h
`-- 1.0/
    |-- cmsis_os1.h
    `-- cmsis_uniproton1.c

testsuites/cmsis/
|-- common/
|   `-- cmsis_test_log.h
`-- v1/
    |-- cmsis_v1_basic_test.c
    |-- cmsis_v1_sem_e2e_test.c
    |-- cmsis_v1_mutex_e2e_test.c
    |-- cmsis_v1_msgq_e2e_test.c
    |-- cmsis_v1_signal_e2e_test.c
    |-- cmsis_v1_timer_e2e_test.c
    |-- cmsis_v1_priority_e2e_test.c
    `-- cmsis_v1_suspend_resume_e2e_test.c
```

## 2 编译控制

组件开关：

```text
CONFIG_OS_SUPPORT_CMSIS=y
```

版本控制只使用`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`：

```c
#define CMSIS_OS_VER 1
```

组件CMake不传递版本宏，只读取`cmsis_os.h`中的`CMSIS_OS_VER=1`并编译`src/component/cmsis/1.0/cmsis_uniproton1.c`。测试套读取同一个头文件并只编译`testsuites/cmsis/v1`目录下的文件。

## 3 支持接口

v1适配层覆盖以下CMSIS-RTOS v1接口类别：

| 类别 | 接口 |
| --- | --- |
| Kernel | `osKernelInitialize`、`osKernelStart`、`osKernelRunning`、`osKernelSysTick` |
| Thread | `osThreadCreate`、`osUsrThreadCreate`、`osThreadGetId`、`osThreadTerminate`、`osThreadSelfSuspend`、`osThreadResume`、`osThreadYield`、`osThreadSetPriority`、`osThreadGetPriority` |
| Signal | `osSignalSet`、`osSignalClear`、`osSignalWait` |
| Delay/Wait | `osDelay`、`osWait` |
| Timer | `osTimerCreate`、`osTimerStart`、`osTimerStop`、`osTimerRestart`、`osTimerDelete` |
| Mutex | `osMutexCreate`、`osMutexWait`、`osMutexRelease`、`osMutexDelete` |
| Semaphore | `osSemaphoreCreate`、`osBinarySemaphoreCreate`、`osSemaphoreWait`、`osSemaphoreRelease`、`osSemaphoreDelete` |
| Pool | `osPoolCreate`、`osPoolAlloc`、`osPoolCAlloc`、`osPoolFree`、`osPoolDelete` |
| Message Queue | `osMessageCreate`、`osMessagePut`、`osMessagePutHead`、`osMessageGet`、`osMessageDelete` |
| Mail Queue | `osMailCreate`、`osMailAlloc`、`osMailCAlloc`、`osMailPut`、`osMailPutHead`、`osMailGet`、`osMailFree`、`osMailClear`、`osMailDelete` |

## 4 关键实现

线程ID通过任务ID加一映射，避免任务ID 0被误判为NULL：

```c
static osThreadId CmsisHandleToThreadId(TskHandle taskId)
{
    return (osThreadId)(uintptr_t)(taskId + 1U);
}
```

v1优先级和UniProton优先级方向相反，适配层使用公共转换函数`CmsisPrioToPrt()`和`CmsisPrioFromPrt()`。`osThreadGetPriority()`返回CMSIS v1优先级枚举，而不是UniProton内部优先级。

v1消息队列使用UniProton queue保存`uint32_t`消息值；mail queue使用queue传递mail指针，mail内存由UniProton内存分配器分配和释放。

v1 signal基于UniProton event实现，`osSignalSet()`向目标任务写事件，`osSignalWait()`使用`PRT_EventRead()`等待信号位。

## 5 测试设计

v1测试位于`testsuites/cmsis/v1`，统一入口为`cmsis_v1_basic_test.c`中的`cmsis_test()`。测试逻辑只调用CMSIS接口；`cmsis_test_log.h`仅作为sd3403串口日志后端封装`PRT_Printf`。

基础API测试覆盖：

| 测试项 | 覆盖内容 |
| --- | --- |
| `kernel` | 运行状态、tick、start接口 |
| `thread` | 创建、运行、设置/获取优先级、删除 |
| `signal` | 当前线程信号set/wait |
| `semaphore` | 创建、非阻塞wait、release、delete |
| `mutex` | 创建、wait、release、delete |
| `pool` | 分配、清零分配、容量耗尽、释放、删除 |
| `message` | put/get/delete |
| `mail` | alloc/put/get/free/delete |
| `timer` | create/start/callback/delete |
| `delay` | delay后tick推进 |
| `negativePaths` | NULL和非法参数路径 |

端到端测试每个场景一个文件：

| 文件 | 场景 |
| --- | --- |
| `cmsis_v1_sem_e2e_test.c` | 空信号量阻塞等待，release后唤醒waiter |
| `cmsis_v1_mutex_e2e_test.c` | 两线程竞争同一mutex，验证互斥和阻塞唤醒 |
| `cmsis_v1_msgq_e2e_test.c` | producer通过message queue唤醒consumer并保持消息顺序 |
| `cmsis_v1_signal_e2e_test.c` | 线程间signal WaitAll同步 |
| `cmsis_v1_timer_e2e_test.c` | timer回调通过signal唤醒线程 |
| `cmsis_v1_priority_e2e_test.c` | 高低优先级线程等待同一信号量，验证高优先级先被调度 |
| `cmsis_v1_suspend_resume_e2e_test.c` | 线程self suspend后由测试线程resume并继续运行 |

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

预期串口日志前缀为`[cmsis-v1]`，最终通过标志为：

```text
[cmsis-v1][PASS] tick=<tick>
```

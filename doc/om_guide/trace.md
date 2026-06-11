# UniProton Trace使用指南

## 1 Trace功能概述

Trace（追踪）是一种协助分析系统运行过程的功能，在打开后随业务运行过程记录内核各模块的事件信息。诸如任务创建/删除/切换、中断响应、内存分配/释放、信号量/队列/事件量使用等信息，记录在内部缓冲区中以方便后续分析优化。

## 2 Trace总体方案

### 2.1 功能说明

Trace模块通过Hook机制注册到内核各关键路径，当用户调用 `PRT_TraceStart` 启动trace后，内核运行过程中产生的事件会被记录到环形缓冲区。Trace模块提供以下能力：

* **事件记录**: 记录系统、中断、任务、软件定时器、内存、队列、事件量、信号量、互斥锁等模块的关键事件
* **自定义事件**: 用户可通过 `PRT_TRACE_EASY` 宏记录自定义事件
* **中断过滤**: 支持注册中断过滤钩子，过滤不需要记录的中断事件
* **事件掩码**: 支持按模块设置事件掩码，选择性记录指定模块的事件
* **离线模式**: 事件记录在内部缓冲区中，停止后可通过Dump接口打印或通过RecordGet接口获取原始数据

### 2.2 Trace模块架构

```
用户应用
   |
   v
PRT_TRACE/PRT_TRACE_EASY  (事件触发宏)
   |
   v
g_traceEventHook  (事件钩子)
   |
   v
OsTraceHook  (核心事件处理)
   |
   v
OsTraceWriteOrSendEvent  (写入离线缓冲区)
   |
   v
TraceOfflineHeaderInfo  (离线缓冲区)
   |
   v
PRT_TraceRecordDump / PRT_TraceRecordGet  (数据输出)
```

### 2.3 数据结构

#### TraceEventFrame（事件帧）

| 字段 | 说明 |
| ---- | ---- |
| eventType | 事件类型，包含模块标志位和事件编号 |
| curTask | 当前任务ID（含掩码版本） |
| curTime | 事件发生时的CPU时钟周期数 |
| identity | 事件标识 |
| core | 核心信息（cpuId、hwiActive、taskLockCnt等，需开启OS_OPTION_TRACE_FRAME_CORE_MSG） |
| eventCount | 事件计数（需开启OS_OPTION_TRACE_FRAME_EVENT_COUNT） |
| params | 事件参数数组，长度由OS_TRACE_FRAME_MAX_PARAMS配置 |

#### TraceObjData（对象数据）

| 字段 | 说明 |
| ---- | ---- |
| id | 对象ID（任务ID等） |
| prio | 对象优先级 |
| name | 对象名称 |

#### TraceOfflineHead（离线缓冲区头）

| 字段 | 说明 |
| ---- | ---- |
| baseInfo.bigLittleEndian | 大小端标识（0x12345678） |
| baseInfo.clockFreq | 时钟频率 |
| baseInfo.version | Trace版本号（与模式相关） |
| totalLen | 缓冲区总长度 |
| objSize | TraceObjData结构大小 |
| frameSize | TraceEventFrame结构大小 |
| objOffset | 对象数据偏移 |
| frameOffset | 事件帧数据偏移 |

### 2.4 事件类型

Trace支持以下模块的事件记录：

| 模块 | 标志位 | 事件 |
| ---- | ---- | ---- |
| 系统(SYS) | 0x10 | SYS_ERROR, SYS_START, SYS_STOP |
| 中断(HWI) | 0x20 | HWI_CREATE, HWI_DELETE, HWI_RESPONSE_IN, HWI_RESPONSE_OUT, HWI_ENABLE, HWI_DISABLE |
| 任务(TASK) | 0x40 | TASK_CREATE, TASK_PRIOSET, TASK_DELETE, TASK_SUSPEND, TASK_RESUME, TASK_SWITCH |
| 软件定时器(SWTMR) | 0x80 | SWTMR_CREATE, SWTMR_DELETE, SWTMR_START, SWTMR_STOP, SWTMR_EXPIRED |
| 内存(MEM) | 0x100 | MEM_ALLOC, MEM_ALLOC_ALIGN, MEM_REALLOC, MEM_FREE, MEM_INFO_REQ, MEM_INFO |
| 队列(QUEUE) | 0x200 | QUEUE_CREATE, QUEUE_DELETE, QUEUE_RW |
| 事件(EVENT) | 0x400 | EVENT_CREATE, EVENT_DELETE, EVENT_READ, EVENT_WRITE, EVENT_CLEAR |
| 信号量(SEM) | 0x800 | SEM_CREATE, SEM_DELETE, SEM_PEND, SEM_POST |
| 互斥锁(MUX) | 0x1000 | MUX_CREATE, MUX_DELETE, MUX_PEND, MUX_POST |

### 2.5 Hook注册

Trace模块通过内核Hook机制注册到以下关键路径：

* OS_HOOK_TSK_SWITCH: 任务切换
* OS_HOOK_TSK_CREATE: 任务创建
* OS_HOOK_TSK_DELETE: 任务删除
* OS_HOOK_HWI_ENTRY: 中断进入
* OS_HOOK_HWI_EXIT: 中断退出

注：开启Trace时，Hook模块会在原有配置数量上为各相关Hook增加1个槽位。

## 3 编译时使能Trace功能

### 3.1 defconfig配置

在构建配置的defconfig文件中设置：

```
CONFIG_OS_OPTION_TRACE=y                              # Trace功能总开关

# 以下两个宏控制TraceEventFrame中是否包含额外信息字段
# CONFIG_OS_OPTION_TRACE_FRAME_CORE_MSG is not set    # 是否包含核心信息(cpuId等)
# CONFIG_OS_OPTION_TRACE_FRAME_EVENT_COUNT is not set # 是否包含事件计数

# Trace缓冲区配置
CONFIG_OS_TRACE_BUFFER_SIZE=4096                      # Trace离线缓冲区大小（字节）
CONFIG_OS_TRACE_OBJ_MAX_NUM=16                        # Trace最大记录对象数
CONFIG_OS_TRACE_FRAME_MAX_PARAMS=3                    # Trace帧最大参数数
```

### 3.2 prt_config.h配置

Trace功能的缓冲区参数（`OS_TRACE_BUFFER_SIZE`、`OS_TRACE_OBJ_MAX_NUM`、`OS_TRACE_FRAME_MAX_PARAMS`）已通过defconfig自动生成到 `prt_buildef.h` 中，无需在 `prt_config.h` 中重复定义。功能开关也通过 `OS_OPTION_TRACE`（由defconfig中的 `CONFIG_OS_OPTION_TRACE=y` 生成）控制，无需额外定义 `OS_INCLUDE_TRACE`。

注：开启Trace时，Hook模块会在原有配置数量上通过 `OsMhookReserve` 为各相关Hook动态预留1个槽位，无需在 `prt_config.h` 中调整 `OS_HOOK_*_NUM` 的值。

### 3.3 模块注册

在 `demos/<board>/config/prt_config.c` 的模块配置表中，Trace模块通过以下配置注册到系统（当 `OS_OPTION_TRACE` 使能时自动生效）：

```c
#if defined(OS_OPTION_TRACE)
    extern U32 OsTraceConfigReg(void);
    extern U32 OsTraceConfigInit(void);
#endif

// 在 g_moduleConfigTab 中注册
#if defined(OS_OPTION_TRACE)
    {OS_MID_TRACE, {OsTraceConfigReg, OsTraceConfigInit}},
#endif
```

## 4 Trace功能接口使用

### 4.1 PRT_TraceStart

```c
U32 PRT_TraceStart(void);
```

启动Trace功能。返回值为 `OS_OK` 表示成功，返回 `OS_ERRNO_TRACE_ERROR_STATUS` 表示Trace未初始化。

### 4.2 PRT_TraceStop

```c
void PRT_TraceStop(void);
```

停止Trace功能，停止后可以使用RecordDump或RecordGet接口获取缓冲区数据。

### 4.3 PRT_TraceEventMaskSet

```c
void PRT_TraceEventMaskSet(U32 mask);
```

设置事件掩码，控制哪些模块的事件会被记录。mask参数为各模块标志位的组合，例如设置只记录任务和中断事件：

```c
PRT_TraceEventMaskSet(TRACE_HWI_FLAG | TRACE_TASK_FLAG);
```

### 4.4 PRT_TraceReset

```c
void PRT_TraceReset(void);
```

重置Trace缓冲区，清空已记录的事件帧数据。

### 4.5 PRT_TraceRecordDump

```c
void PRT_TraceRecordDump(bool toClient);
```

打印Trace缓冲区中的事件和对象数据。toClient参数保留，当前默认打印到串口。

### 4.6 PRT_TraceRecordGet

```c
TraceOfflineHead *PRT_TraceRecordGet(void);
```

获取Trace缓冲区头指针，返回的数据可用于用户侧进行自定义解析。

### 4.7 PRT_TraceHwiFilterHookReg

```c
void PRT_TraceHwiFilterHookReg(TraceHwiFilterHook hook);
```

注册中断过滤钩子。回调函数返回true表示过滤该中断事件（不记录），返回false表示记录该中断事件。

```c
static bool MyHwiFilter(U32 hwiNum)
{
    // 过滤中断号1，不记录
    return (hwiNum == 1);
}

PRT_TraceHwiFilterHookReg(MyHwiFilter);
```

### 4.8 PRT_TRACE宏

```c
#define PRT_TRACE(TYPE, IDENTITY, ...)
```

记录指定类型的事件。TYPE为预定义的事件类型枚举值，IDENTITY为事件标识，后续参数为事件相关参数。

```c
// 记录中断响应事件
PRT_TRACE(HWI_RESPONSE_IN, hwiNum);

// 记录任务创建事件
PRT_TRACE(TASK_CREATE, taskId, taskStatus, priority);

// 记录信号量创建事件
PRT_TRACE(SEM_CREATE, semId, type, count);
```

### 4.9 PRT_TRACE_EASY宏

```c
#define PRT_TRACE_EASY(TYPE, IDENTITY, ...)
```

记录自定义用户事件，事件类型自动标记为 `TRACE_USER_DEFAULT_FLAG | TYPE`。

```c
// 记录自定义事件
PRT_TRACE_EASY(0x01, 0x12345678, param1, param2);
PRT_TRACE_EASY(0x02, 0xABCDEFFF, param1, param2, param3);
```

注：当 `OS_OPTION_TRACE` 未使能时，`PRT_TRACE` 和 `PRT_TRACE_EASY` 宏展开为空，不会产生任何代码。

## 5 Trace功能使用示例

```c
#include "prt_trace_external.h"
#include "prt_task.h"

void TraceTest(void)
{
    U32 ret;
    TraceOfflineHead *head;

    // 1. 启动Trace
    ret = PRT_TraceStart();
    if (ret != OS_OK) {
        return;
    }

    // 2. 设置事件掩码（记录所有模块）
    PRT_TraceEventMaskSet(0xFFFFFFFF);

    // 3. 记录事件
    PRT_TRACE(TASK_CREATE, 1, 0x04, 10);
    PRT_TRACE(SEM_CREATE, 0, 0, 1);
    PRT_TRACE(MEM_ALLOC, 0x1000, 0x2000, 128);
    PRT_TRACE_EASY(0x01, 0x12345678, 0x01, 0x02);

    // 4. 停止Trace
    PRT_TraceStop();

    // 5. Dump打印数据
    PRT_TraceRecordDump(FALSE);

    // 6. 获取原始数据
    head = PRT_TraceRecordGet();
    if (head != NULL) {
        // 分析 head->baseInfo, head->totalLen 等字段
    }

    // 7. 重置缓冲区
    PRT_TraceReset();
}
```

## 6 测试设计

### 6.1 测试用例位置

Trace测试用例位于 `testsuites/trace-test/` 目录下：

```
testsuites/trace-test/
├── trace_test.c            # 测试主文件
├── trace_test_public.h     # 测试公共头文件
└── CMakeLists.txt          # 测试CMake构建文件
```

### 6.2 测试用例说明

测试用例覆盖以下功能点：

| 序号 | 测试项 | 说明 |
| ---- | ---- | ---- |
| 1 | Trace Start | 验证Trace正常启动 |
| 2 | Event Mask Set | 验证事件掩码设置 |
| 3 | PRT_TRACE_EASY | 验证自定义事件记录功能 |
| 4 | PRT_TRACE | 验证各模块标准事件记录功能 |
| 5 | Trace Record Get | 验证TraceRecordGet接口返回值 |
| 6 | Trace Stop | 验证Trace停止功能 |
| 7 | Trace Record Dump | 验证数据Dump打印功能 |
| 8 | Trace Reset | 验证缓冲区重置功能 |
| 9 | Trace Restart | 验证Trace停止后重新启 |
| 10 | HWI Filter Hook | 验证中断过滤钩子注册 |

### 6.3 编译运行测试

#### 6.3.1 构建命令

在构建容器内执行：

```bash
cd /home/uniproton/demos/sd3403/build
sh build_app.sh UniPorton_test_trace
```

构建产物为 `UniPorton_test_trace.elf`。

#### 6.3.2 上板运行

将elf文件拷贝到测试板后，通过mica启动：

```bash
# 上传ELF文件
scp UniPorton_test_trace.elf root@192.168.7.2:/etc/mica/

# 配置mica
echo '[Mica]' > /etc/mica/sd3403.conf
echo 'Name=sd3403' >> /etc/mica/sd3403.conf
echo 'CPU=3' >> /etc/mica/sd3403.conf
echo 'ClientPath=/etc/mica/UniPorton_test_trace.elf' >> /etc/mica/sd3403.conf
echo 'AutoBoot=no' >> /etc/mica/sd3403.conf

# 离线CPU3
echo 0 > /sys/devices/system/cpu/cpu3/online

# 清理旧实例
mica stop sd3403 2>/dev/null; mica rm sd3403 2>/dev/null

# 启动串口日志采集（串口设备为 /dev/ttyUSB0）
# 创建并启动实例
mica create /etc/mica/sd3403.conf
mica start sd3403

# 等待测试输出...
# 测试完成后查看串口输出
```

#### 6.3.3 预期结果

串口输出应包含如下内容：

```
[trace] === Trace Feature Test Start ===
[trace] Test 1: Trace Start
[trace] Trace is running
[trace] Test 2: Set event mask to all modules
[trace] Event mask set
[trace] Test 3: Use PRT_TRACE_EASY macro
[trace] PRT_TRACE_EASY events recorded
[trace] Test 4: Use PRT_TRACE macro
[trace] PRT_TRACE events recorded
[trace] Test 5: Get trace record
[trace] Trace buffer acquired OK
[trace] clockFreq = XXX, version = XXX, totalLen = XXX
[trace] objSize = XXX, frameSize = XXX, objOffset = XXX, frameOffset = XXX
[trace] Test 6: Stop trace
[trace] Trace stopped
[trace] Test 7: Dump trace records
[trace] Trace dump completed
*******TraceInfo begin*******
... (事件数据) ...
*******TraceInfo end*******
[trace] Test 8: Reset trace buffer
[trace] Trace buffer reset
[trace] Test 9: Restart trace
[trace] Trace restarted OK
[trace] Test 10: HWI filter hook
[trace] HWI filter hook registered
[trace] === ALL TRACE TESTS PASSED ===
```

## 7 注意事项

### 7.1 性能影响

Trace模块在事件发生时需要同步写入缓冲区，会产生一定的性能开销。建议：
* 仅在调试分析时开启Trace功能
* 通过事件掩码只记录关心的模块事件
* 合理设置缓冲区大小，避免内存浪费

### 7.2 钩子数量

开启Trace后，Trace模块通过 `OsMhookReserve` 为以下各钩子动态预留1个槽位：
* OS_HOOK_HWI_ENTRY
* OS_HOOK_HWI_EXIT
* OS_HOOK_TSK_CREATE
* OS_HOOK_TSK_DELETE
* OS_HOOK_TSK_SWITCH

如果用户的 `prt_config.h` 中已配置了较大的钩子数量，可正常使用；如果钩子数量设置较小，需要确保预留空间足够。

### 7.3 缓冲区大小

Trace缓冲区通过 `PRT_MemAlloc(OS_MID_TRACE, OS_MEM_DEFAULT_FSC_PT, size)` 从内存池分配，需要确保内存池有足够的空间。缓冲区过小可能导致事件记录被覆盖（环形缓冲区特性，覆盖旧数据）。

### 7.4 中断上下文安全

Trace的事件记录接口（`PRT_TraceStart`、`PRT_TraceStop` 除外）使用关中断保护临界区，可在中断上下文中安全调用。

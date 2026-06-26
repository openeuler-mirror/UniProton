# UniProton 低功耗（Low-power）设计文档

适用范围：`src/extended/lowpower/`，目标板 `demos/sd3403`（armv8-a，EL1，单核非 SMP，mica 实例内运行）。
流程来源：LiteOS `los_lowpower.c` / `los_lowpower_impl.c` / `los_runstop.c` / `los_deepsleep_pri.h` / `arch/arm64/src/runstop.S`。**流程不变，仅把内核接口替换为 UniProton `PRT_` 等价接口。**

---

## 1. 概述

低功耗框架提供三类能力，均按 LiteOS 流程实现：

| 能力 | 对应 LiteOS | 说明 |
|---|---|---|
| PowerMgr（电源管理） | `los_lowpower_impl.c` | idle 钩子驱动的 light/deep sleep 选择、频点切换、深睡投票（vote） |
| Runstop（运行-停止/快照） | `los_runstop.c` | 把真实 RAM 镜像（`.data/.bss/寄存器/tick`）存到后端，深睡后恢复 |
| 寄存器上下文 | `los_deepsleep_pri.h` + `runstop.S` | `OsLowpowerSaveRegister` / `OsLowpowerRestoreRegister` |
| Tickless（适配层） | `los_tickless.c` | LiteOS tickless 适配；**已移植到 non-SMP、关/开均验证 PASS（见 §15）** |

sd3403 当前活跃路径：**light-sleep + runstop**。`DEEPSLEEP` 关闭（无 BSP 真深睡驱动）。`TICKLESS` 已移植到 non-SMP：关/开均板上验证 PASS（swtmr one-shot `+20`、loop `10 10…`、runstop PASS）；默认关以保持 0 work-mode 刷屏，可开（详见 §15）。

---

## 2. 架构

### 2.1 模块树

```
src/include/uapi/                      # 公共 API 头
  prt_lowpower.h          # PowerMgrOps / PRT_LowpowerInit / idle+wakeup 钩子注册
  prt_lowpower_impl.h     # PowerMgrRunOps / PowerMgrDeepSleepOps / PowerMgrParameter / PRT_PowerMgrInit
  prt_lowpower_context.h  # g_lowpowerSaveSRContext / Save|RestoreRegister / resume 标志
  prt_runstop.h           # RunstopStorageOps / RunstopLowpowerOps / RunstopImageInfo / WowShmHeader
  prt_tickless.h          # tickless 适配 API（TICKLESS 开时编入）

src/extended/lowpower/
  prt_lowpower.c          # 框架：idle 钩子分发、int-wakeup 钩子存储、PowerMgrOps 转发
  prt_lowpower_context.c  # g_lowpowerSaveSRContext[34] / g_lowpowerAR[3] / resume 标志
  arch/armv8/
    prt_lowpower_context_asm.S  # OsLowpowerSaveRegister / OsLowpowerRestoreRegister（EL1）
  powermgr/
    prt_lowpower_impl.c   # OsLowpowerProcess（idle 主体）/ 投票 / light+deep sleep / 频点
  runstop/
    prt_runstop.c         # OsRunstopSuspend（成像+light）/ OsRunstopMakeImage / OsRunstopSystemWakeup
  tickless/
    prt_tickless.c        # tickless 适配（TICKLESS 开时编入；已修 non-SMP 编译/运行）

src/arch/cpu/armv8/common/hwi/prt_hwi.c   # OsHwiDispatchHandle 内调 OsLowpowerIntWakeupHookCall(hwiNum)
demos/sd3403/bsp/
  sd3403/cpu_config.h     # WOW_IMG_ADDR / WOW_IMG_SIZE / OpenAMP / log / image 地址
  mmu.c                   # WOW 镜像区 MMU 映射
testsuites/lowpower-test/ # 上板测试（APP=lowpower）
```

### 2.2 分层与数据流

```
                 ┌──────────────────────────────────────────────┐
  应用/测试  ──> │ PRT_PowerMgr* / OsRunstopMakeImage (uapi)     │
                 └──────────────────────────────────────────────┘
                                   │
                 ┌─────────────────▼──────────────────┐
  idle 钩子  ──> │ prt_lowpower.c  OsPowerMgrProcess   │ ── int-wakeup 钩子 ──> prt_hwi.c
                 │   -> PowerMgrOps.process            │
                 └─────────────────┬──────────────────┘
                                   │
                 ┌─────────────────▼──────────────────┐
                 │ prt_lowpower_impl.c OsLowpowerProcess│
                 │   - runstop 成像? -> OsRunstopStore.. │
                 │   - 否则 light sleep (wfi)            │
                 └─────────────────┬──────────────────┘
                                   │ 成像
                 ┌─────────────────▼──────────────────┐
                 │ prt_runstop.c OsRunstopSuspend       │
                 │   SaveRegister -> saveImage ->        │
                 │   setWakeUpTimer -> enterDeepSleep -> │
                 │   withdraw -> tickCompensate -> post  │
                 └─────────────────┬──────────────────┘
                                   │ 后端
                 ┌─────────────────▼──────────────────┐
                 │ RunstopStorageOps (用户注册)         │
                 │   saveImage/restoreImage/validate..  │
                 │   后端 = WOW_IMG_ADDR (spare mcs_mem)│
                 └─────────────────────────────────────┘
```

---

## 3. 宏控配置（Kconfig）

定义于 `src/extended/Kconfig`，在 `src/Kconfig` 中 `source`。依赖关系：

```
OS_OPTION_LOWPOWER
  ├── OS_OPTION_POWERMGR        (light/deep sleep, freq, voting)
  ├── OS_OPTION_RUNSTOP
  │     └── OS_OPTION_RUNSTOP_WOW_SHM   (镜像走共享内存后端)
  ├── OS_OPTION_DEEPSLEEP       (reset 后寄存器恢复；sd3403 关)
  └── (OS_OPTION_TICKLESS)      # 原生宏，在 core/Kconfig；已移植 non-SMP，默认关可开
```

| 宏 | 作用 | sd3403 当前 | 备注 |
|---|---|---|---|
| `CONFIG_OS_OPTION_LOWPOWER` | 低功耗框架总开关，编入 `prt_lowpower.c` + context | `y` | 必开 |
| `CONFIG_OS_OPTION_POWERMGR` | `prt_lowpower_impl.c`，idle 主体 + 投票 | `y` | 依赖 LOWPOWER |
| `CONFIG_OS_OPTION_RUNSTOP` | `prt_runstop.c`，快照成像/恢复 | `y` | 依赖 LOWPOWER |
| `CONFIG_OS_OPTION_RUNSTOP_WOW_SHM` | 镜像走共享内存 carveout 后端 | `y` | 依赖 RUNSTOP |
| `CONFIG_OS_OPTION_DEEPSLEEP` | reset 后寄存器恢复路径 | 关 | 依赖 LOWPOWER；无 BSP 驱动 |
| `CONFIG_OS_OPTION_TICKLESS` | tickless 适配 | 默认关 | 已移植 non-SMP、关/开均验证；开则 managed sleep，补偿/swtmr 模型见 §15 |

当前 `build/uniproton_config/config_armv8_sd3403/defconfig`（节选）：

```
CONFIG_OS_OPTION_LOWPOWER=y
CONFIG_OS_OPTION_POWERMGR=y
CONFIG_OS_OPTION_RUNSTOP=y
CONFIG_OS_OPTION_RUNSTOP_WOW_SHM=y
# CONFIG_OS_OPTION_DEEPSLEEP is not set
# CONFIG_OS_OPTION_TICKLESS is not set
```

编译门控见 `src/extended/lowpower/CMakeLists.txt`：框架 + context 在 LOWPOWER 时常编入；`tickless/runstop/powermgr` 子目录各自按其宏决定；armv8 汇编在 `CONFIG_OS_ARCH_ARMV8` 下编入。

---

## 4. 初始化流程

板级/应用初始化顺序（参考 `testsuites/lowpower-test/lowpower_test.c` 的 `lowpower_test()`）：

```
1. OsLowpowerContextInit()                 # 清零 g_lowpowerSaveSRContext / resume 标志
2. PRT_PowerMgrInit(NULL)                  # 注册 g_pmOps（含 OsLowpowerProcess），
                                           #   并经 PRT_LowpowerInit 注册：
                                           #     - idle 钩子 OsPowerMgrProcess
                                           #     - 中断唤醒钩子 OsPowerMgrWakeUpFromInterrupt
3. OsRunstopStorageOpsRegister(&ops)       # 注册 saveImage/restoreImage/validateImage
4. OsRunstopSetSleepTicks(N)               # 设定模拟深睡时长（tick）
5. OsRunstopInit()                         # OsRunstopStateInit + OsRunstopBackendInit
                                           #   + OsRunstopResumeCheck (冷启动跳过)
                                           #   + OsWowWriteFlashTaskCreate (成像专用任务)
```

`PRT_PowerMgrInit(NULL)` 传 `NULL` 表示用默认 `PowerMgrRunOps`（`OsLightSleepDefault=wfi`、`OsEnterDeepSleepDefault=wfi`、`getSleepTime` 在无 tickless 时返回 0、有 tickless 时返回真实睡眠预算）。BSP 可传 `struct PowerMgrParameter` 覆盖部分 op。`PRT_PowerMgrInit`→`PRT_LowpowerInit` 在 `#if TICKLESS` 下还调 `PRT_TicklessEnable()`（对应 LiteOS `los_init` 的 `LOS_TicklessEnable`），并接上 idle 的 `g_taskCoreSleep=wfi`。

`OsRunstopInit` 内若 `OsRunstopResumeCheck()==RUNSTOP_RESUME`（真复位恢复），会先调 `OsRunstopSystemWakeup()` 恢复镜像；冷启动则跳过。

---

## 5. 核心数据结构与 API

### 5.1 PowerMgrOps（`prt_lowpower.h`）— 框架转发层

```c
struct PowerMgrOps {
    void (*process)(void);               // idle 主体（OsLowpowerProcess）
    void (*wakeupFromReset)(void);       // 复位恢复入口
    void (*resumeFromInterrupt)(U32);    // 中断唤醒
    void (*changeFreq)(enum PRT_FreqMode);
    void (*deepSleepVoteBegin)(void);
    void (*deepSleepVoteEnd)(void);
    void (*deepSleepVoteDelay)(U32 tick);
    void (*registerExternalVoter)(LowpowerExternalVoterHandle);
    U32  (*getDeepSleepVoteCount)(void);
    U32  (*getSleepMode)(void);
    void (*setSleepMode)(U32 mode);
};
```

应用侧投票 API：`PRT_PowerMgrDeepSleepVoteBegin/End`、`PRT_PowerMgrGetDeepSleepVoteCount`、`PRT_PowerMgrSleepDelay`、`PRT_PowerMgrRegisterExtVoter`、`PRT_PowerMgrChangeFreq`。

### 5.2 RunstopStorageOps（`prt_runstop.h`）— 镜像后端（用户实现）

```c
struct RunstopStorageOps {
    U32 (*getImageInfo)(RunstopImageInfo *info);  # 可选，后端可回填地址/大小
    U32 (*validateImage)(void);                   # 复位恢复前校验 magic
    U32 (*saveImage)(const RunstopImageInfo *info);# 存 .data/.bss/SR/tick
    void (*restoreImage)(void);                   # 复位恢复时回填 RAM
};
```

```c
typedef struct {
    U32 imageSize;
    U64 dataStart, dataSize;     # __data_start .. __data_end
    U64 bssStart,  bssSize;      # &__bss_start__ .. &__bss_end__
    U64 srContextAddr, srContextSize;  # g_lowpowerSaveSRContext, 272B
    U64 tickCount;               # PRT_TickGetCount() 挂起时刻
} RunstopImageInfo;
```

### 5.3 RunstopLowpowerOps（`prt_runstop.h`）— 深睡钩子（用户可选）

```c
struct RunstopLowpowerOps {
    void (*enterDeepSleep)(void);      # NULL => 默认 WFI 循环（g_runstopSleepTicks 次）
    void (*setWakeUpTimer)(U32 ticks); # 配唤醒源
    void (*withdrawWakeUpTimer)(void); # 撤唤醒源
};
```

默认 `enterDeepSleep` = `OsRunstopEnterDeepSleepDefault`（WFI 循环，被硬件 tick 唤醒），对应 LiteOS `OsEnterDeepSleepDefault = wfi()`。

### 5.4 寄存器上下文（`prt_lowpower_context.h` + asm）

```c
extern U64 g_lowpowerSaveSRContext[34];  # X0-X29、SP、LR、DAIF、NZCV（272B）
extern U64 g_lowpowerSaveAR[3];          # 跨例程暂存 X0/X1/X2
extern volatile U32 g_lowpowerResumeFromImg;  # LOWPOWER_COLD_RESET / RUNSTOP_RESET / DEEP_SLEEP_RESET
```

`OsLowpowerSaveRegister`：把当前 CPU 上下文存入 `g_lowpowerSaveSRContext`（EL1，故 `SPSR_ELx=spsr_el1`、`ELR_ELx=elr_el1`，`RUNLVL=4`）。
`OsLowpowerRestoreRegister`：从 `g_lowpowerSaveSRContext` 恢复，并经 `SPSR_EL1/ELR_EL1` + `eret` 风格回到 `OsLowpowerSaveRegister` 的调用点。**仅在复位恢复路径调用。**

---

## 6. 运行时流程

### 6.1 idle → PowerMgr（`OsLowpowerProcess`，`prt_lowpower_impl.c`）

```
idle 钩子 OsPowerMgrProcess -> OsLowpowerProcess:
  if OsRunstopWowSysDoneFlagGet()==OS_STORE_SYSTEM:   # 有成像请求
      OsRunstopStoreSystemInfoBeforeSuspend()         # core0 post writeFlashSem
                                                        #   -> OsWowWriteFlashTask -> OsRunstopSuspend
  OsChangeFreq()                       # 频点切换（若有 pending）
  #── 无 TICKLESS 且无 DEEPSLEEP 时到此早返回，wfi 交 g_taskCoreSleep（见 §11.1）──
  #── 以下 managed 路径仅在 #if TICKLESS||DEEPSLEEP 编入 ──
  TaskLock + IntLock
  sleepTicks = getSleepTime()           # 无 tickless=>0；有 tickless=>真实预算
  if sleepTicks<=minSleepTicks 或 voteCount!=0:
      postConfig; (tickless open); wfi  # work mode（近事件/投票阻止）
  else:
      mode = selectSleepMode(sleepTicks)
      if mode>=DEEP_SLEEP: deepSleep(...)   # DEEPSLEEP 关闭时回落 light
      postConfig
      if mode<DEEP_SLEEP: lightSleep(...)   # tickless open + enterLightSleep(wfi)
  TaskUnlock + IntRestore
```

> **两种配置下的 idle 行为**：TICKLESS 关时 `getSleepTime()` 恒 0，`OsLowpowerProcess` 在 runstop 桥+频点后**早返回**，wfi 交 `g_taskCoreSleep`（无 work-mode 打印、无 per-idle 锁开销）。TICKLESS 开时走 managed 路径：`getSleepTime()` 返回真实预算，>minSleepTicks 则 managed light-sleep（`OsTicklessOpen` 重装 one-shot + wfi），≤minSleepTicks 则 work-mode（`wfi` + "work mode" 打印，LiteOS 行为，近事件时命中）。成像请求到达时优先走 runstop 分支。

### 6.2 Runstop 成像 suspend（`OsRunstopSuspend`，LiteOS `OsSystemSuspend`）

两分支：

**成像分支**（`IsImageResume()` 真，由 `OsRunstopMakeImage` 经 idle 钩子触发）：

```
TaskLock + IntLock
1. OsLowpowerSaveRegister()                      # 存寄存器上下文
2. fillImageInfo + storageOps->saveImage()       # 存 .data/.bss/SR/tick 真实快照
3. tickBefore = TickGetCount()
   setWakeUpTimer(sleepTicks)
   enterDeepSleep()                              # 默认 WFI 循环 sleepTicks 次
   withdrawWakeUpTimer()
   tickAfter = TickGetCount()
4. elapsed = tickAfter - tickBefore
   if sleepTicks > elapsed: TickCountAdjust(sleepTicks - elapsed)   # OsSysTimeUpdate
5. result = FLASH_IMG_SUCCESS; postSem=true
TaskUnlock + IntRestore
if postSem: SemPost(g_suspendResumeSem)          # OsWaitImagingDone 握手
```

**light 分支**（无成像请求，LiteOS `OsLightSleepDefault`）：

```
wfi()                       # 单次，等到下一个中断
result = WAKEUP_FROM_SUSPEND
# 不 post suspendResumeSem（无 MakeImage 等待方，避免误唤醒）
```

### 6.3 MakeImage 握手（`OsRunstopMakeImage`，LiteOS `LOS_MakeImage`）

```
g_lowpowerResumeFromImg = LOWPOWER_RUNSTOP_RESET
g_sysDoneFlag[core] = OS_STORE_SYSTEM
SemPend(g_suspendResumeSem, FOREVER)   # 等 OsRunstopSuspend 成像分支 post
ret = g_runstopResult
g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET
return ret                              # 期望 FLASH_IMG_SUCCESS
```

调用方（测试任务）阻塞在 sem 上；idle 钩子看到 `OS_STORE_SYSTEM` 后经 `OsWowWriteFlashTask` 执行 `OsRunstopSuspend` 成像分支，完成后 post sem 唤醒调用方。**全程单 mica 实例，Linux 无感。**

### 6.4 复位恢复（真深睡，预留；LiteOS `OsSystemWakeup`）

```
boot/reset -> OsLowpowerWakeupFromReset:
  if g_lowpowerResumeFromImg==LOWPOWER_RUNSTOP_RESET:
      OsRunstopSystemWakeup():
          storageOps->restoreImage()           # 回填 .data/.bss
          g_lowpowerResumeFromImg = COLD_RESET
          g_lowpowerOtherCoreResume = 1
          OsLowpowerRestoreRegister()          # eret 回到 OsLowpowerSaveRegister 调用点
```

> 当前 wfi 模拟路径 **不调用** `OsLowpowerRestoreRegister`（RAM 保留，inline 续跑）。这与 LiteOS 一致：`OsSRRestoreRegister` 只在复位恢复路径，不在默认 wfi 路径。

---

## 7. 内存布局（sd3403）

WOW 镜像区选在 **spare mcs_mem**（`/proc/iomem` 中 `43000000-46ffffff`），避开 OpenAMP 通信区与 log 区：

| 区域 | 起址 | 大小 | 用途 |
|---|---|---|---|
| OpenAMP SHM | `0x40000000` | `0x100000` | 通信区，**不可占用** |
| Image/MMU | `0x43000000` | `0x1000000` | RTOS 镜像 + 页表 |
| Log mem | `0x44000000` | `0x1100000` | 日志区 |
| **WOW 镜像** | **`0x45200000`** | **`0x200000`(2MB)** | runstop 快照后端 |

> `0x44000000 + 0x1100000 = 0x45100000`，向上 2MB 对齐到 `0x45200000`，落在 log 之后、`46ffffff` 之前。

MMU 映射（`demos/sd3403/bsp/mmu.c`）：`virt=phys=0x45200000`，`MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX`（共享 carveout，跨核/跨环境可见）。

镜像内布局（`testsuites/lowpower-test/lowpower_test.c` 的 `TestSaveImage`）：

```
[WOW_IMG_ADDR]:
  WowShmHeader{ magic=0x57574F57, imageSize, dataSize, bssSize, resumeFlag }
  .data   (dataSize 字节)
  .bss    (bssSize 字节)
  SR 上下文 (272B = 34*U64)
  tick    (8B)
```

---

## 8. 测试用例

代码：`testsuites/lowpower-test/lowpower_test.{c,h}` + `CMakeLists.txt`。在 `demos/sd3403/apps/openamp/main.c` 的 `LOWPOWER_TESTCASE` 下调用 `lowpower_test()`。

### 8.1 构建

```bash
# 容器内
cd /home/uniproton/UniProton/demos/sd3403/build
sh build_app.sh lowpower          # APP=lowpower，产物 lowpower.elf / lowpower.bin
```

### 8.2 上板

部署脚本 `/tmp/opencode/deploy_lowpower.py`（scp 镜像、mica 启动 instance、抓串口日志、清理）。日志落 `demos/sd3403/build/test-results.txt`。

### 8.3 用例与预期

| 用例 | 校验点 | 预期 |
|---|---|---|
| PowerMgr 投票 | `vote 0→2(begin×2)→1(end×1)` | 计数正确 |
| Tickless | 关：早返回+`g_taskCoreSleep=wfi`；开：managed sleep，`OsSleepTicksGet`=budget，swtmr 扫描切到 expectedTick 绝对扫描 | 关:`option disabled`、0 work-mode；开:`OsSleepTicksGet=70`、loop `10 10…`、one-shot `+20`（详见 §15） |
| Sw timer（swtmr guard） | 单次 200ms 定时器在 ≈20 tick 触发 | `fired at +20 ticks`（关/开均需通过） |
| Runstop 成像 | `OsRunstopMakeImage` 返回 `FLASH_IMG_SUCCESS`；挂起 ≈ `SUSPEND_TICKS`(30) tick | `suspended ~31 ticks (inline resume)` |
| saveImage 回调 | `g_saveImageCalled==true` | 回调被调用 |
| validateImage | magic 校验 | `RUNSTOP_OK` |
| Readback `.bss` | 保存的 `g_testMarker==live` | `0xa5a55a5a == 0xa5a55a5a` |
| Readback `.data` | 保存的 `g_testDataVar==live` | `0x123456789abcdef0 == ...` |
| Readback tick | 保存的挂起时刻 tick < 当前 tick | `saved=641 current=672` |

通过判定：`[LOWPOWER] RESULT: PASS`（0 failures）。

### 8.4 验证模型约束

- **单次启动、Linux 无感**：同一 mica instance 内，WFI 模拟深睡 + 硬件 tick 唤醒 + inline 续跑；**不** mica stop/start 二次启动、**不** reset vector、**不** PSCI。
- 镜像为**真实快照**（`.data/.bss/SR/tick`），非 dummy buffer；readback 逐项比对 live RAM 一致。

---

## 9. 用户约束

### 9.1 流程约束

1. **必须按 LiteOS 流程**，不得私自改步骤顺序。已对位的 LiteOS 函数见各源文件头注释（`OsSystemSuspend` / `OsWriteToFlashTask` / `OsWaitImagingDone` / `OsStoreSystemInfoBeforeSuspend` / `OsSystemWakeup` / `OsLightSleepDefault` / `OsEnterDeepSleepDefault` / `OsSRSaveRegister` / `OsSRRestoreRegister` / `OsSysTimeUpdate`）。
2. **仅替换内核接口为 UniProton `PRT_`**：如 LiteOS `LOS_Atomic*` → `PRT_Atomic*`、`LOS_Spinlock*` → `PrtSpinLock`、`LOS_Event*`（独立 CB）→ `PRT_Sem*`（计数信号量，因 UniProton event 是 per-task）、`LOS_TaskLock` → `PRT_TaskLock`、`LOS_HwiLock` → `OsIntLock`、`LOS_TickCountAdjust` → `PRT_TickCountAdjust`。
3. **默认深睡 = WFI**（LiteOS `OsEnterDeepSleepDefault`/`OsLightSleepDefault` 均为 `wfi()`）。真深睡由客户经 `RunstopLowpowerOps` 注入。
4. **`OsLowpowerRestoreRegister` 仅在复位恢复路径调用**，wfi 路径不调（RAM 保留）。
5. **`suspendResumeSem` 仅成像分支 post**（`postSem` 标志）。light 分支不得 post，否则后续 `OsRunstopMakeImage` 会误返回。

### 9.2 内存约束

6. **WOW 镜像区不得占用 OpenAMP 通信区**（`0x40000000+0x100000`）或 log 区。sd3403 用 spare mcs_mem `0x45200000`。
7. **`__bss_start__`/`__bss_end__` 是链接符号**，取地址用 `&__bss_start__`，**不得**直接读符号值（那是 .bss 数据，非地址）。`.data` 用 `__data_start[]`（数组名即地址）。参考 `src/arch/cpu/armv8/common/boot/prt_bss_init.c:21`。

### 9.3 已知约束（sd3403）

8. **`CONFIG_OS_OPTION_TICKLESS`：已移植到 non-SMP、关/开均验证 PASS、默认关**。原本 UniProton tickless 是 **SMP-only**（non-SMP 路径未实现/未编过），现已按 LiteOS 流程移植。**完整改动清单、补偿模型推导（为什么 OsTicklessUpdate 对 tick-IRQ 补 `sleepTicks-1`）、swtmr 扫描从 cursor 追赶改为 expectedTick 绝对扫描的原因、关/开对比与板上验证结果，见 §15。** 这里仅列要点：
   - 全部改动 `#if defined(OS_OPTION_TICKLESS)` 门控，**关时零影响**。
   - **编译修复**：`prt_swtmr_internal.h` 加 `OS_SWTMR_SORT_LINK(swtmr)` 双模宏（修 `prt_swtmr_minor.c` 用 SMP-only `coreID`）；`prt_tickless.c` 加 `static` 存储类、`OS_TICK_PER_SECOND` 换运行时 `OsSysGetTickPerSecond()`、include。
   - **运行时移植**：① `PRT_TickISR`（`prt_tick.c`）`#if TICKLESS` 下 `OsTickIrqFlagSet(OsTicklessFlagGet())`；② `PRT_LowpowerInit` `#if TICKLESS` 下 `PRT_TicklessEnable()`；③ `OsSwTmrScan`（`prt_swtmr.c`）`#if TICKLESS` 下改 **expectedTick 绝对扫描**（替代 cursor 追赶，修 loop swtmr 漂移）；④ `OsTicklessUpdate`（`prt_tickless.c`）tick-IRQ 分支补 `sleepTicks-1`（修 off-by-one），并新增 `PRT_TicklessSetTickIrqNum`（BSP 注入 tick IRQ 号，对标 LiteOS `OS_TICK_INT_NUM`）。
   - **已验证**：TICKLESS 开 → one-shot swtmr `+20`、loop swtmr `10 10 10 10 10`、runstop PASS、RESULT PASS。TICKLESS 关 → 0 work-mode 刷屏、one-shot `+20`、runstop PASS（无回归）。
   - **默认关**：靠 §6.1 的设计修复（无 TICKLESS 时 `OsLowpowerProcess` 早返回 + `g_taskCoreSleep` 接 wfi）实现 0 刷屏。开则做 managed sleep（接受近事件 budget<minSleepTicks 时合法 work-mode 诊断打印，LiteOS 行为）。
9. **`CONFIG_OS_OPTION_DEEPSLEEP` 关闭**：无 BSP 真深睡驱动。深睡路径编译裁掉，活跃路径为 light-sleep + runstop。客户要做真深睡需：开启 `DEEPSLEEP` + 实现 `PowerMgrDeepSleepOps` + `RunstopLowpowerOps` + 复位恢复调 `OsRunstopSystemWakeup`/`OsLowpowerRestoreRegister`。
10. **单核非 SMP**（`CONFIG_OS_MAX_CORE_NUM=4` 但 `CONFIG_OS_OPTION_SMP` 关，`CONFIG_OS_THIS_CORE=0`）。多核 resume 路径（`otherCoreResume`/`release_secondary_cores`）预留未启用。

### 9.4 客户扩展点

- **真深睡**：实现 `RunstopLowpowerOps`（`enterDeepSleep` 做掉电/门控 + 配唤醒源），复位向量调 `OsLowpowerWakeupFromReset` → `OsRunstopSystemWakeup`（`restoreImage` + `OsLowpowerRestoreRegister`）。
- **非易失后端**：`RunstopStorageOps` 的 `saveImage/restoreImage` 改为 flash/RAM-in-retain 区实现；`validateImage` 校验 magic。
- **频点/外设**：经 `PowerMgrRunOps.changeFreq` 与 `PowerMgrDeepSleepOps.suspendDevice/resumeDevice` 注入（后者需开 `DEEPSLEEP`）。
- **外部投票方**：`PRT_PowerMgrRegisterExtVoter(handler)`，`handler` 返回非 0 票数则阻止深睡。

---

## 10. 关键文件索引

| 文件 | 行 | 说明 |
|---|---|---|
| `src/include/uapi/prt_lowpower.h` | 46-58 | `PowerMgrOps` |
| `src/include/uapi/prt_lowpower_impl.h` | 25-65 | `PowerMgrRunOps`/`DeepSleepOps`/`Parameter` |
| `src/include/uapi/prt_runstop.h` | 41-111 | `WowShmHeader`/`RunstopImageInfo`/`RunstopStorageOps`/`RunstopLowpowerOps` |
| `src/include/uapi/prt_lowpower_context.h` | 26-36 | resume 标志 + Save/Restore |
| `src/extended/lowpower/prt_lowpower.c` | 75-95 | `PRT_LowpowerInit`（注册 idle + int-wakeup 钩子） |
| `src/extended/lowpower/powermgr/prt_lowpower_impl.c` | 466-513 | `OsLowpowerProcess`（idle 主体） |
| `src/extended/lowpower/powermgr/prt_lowpower_impl.c` | 515-528 | `OsLowpowerWakeupFromReset`（复位恢复分发） |
| `src/extended/lowpower/runstop/prt_runstop.c` | 202-262 | `OsRunstopSuspend`（成像 + light 两分支） |
| `src/extended/lowpower/runstop/prt_runstop.c` | 308-325 | `OsRunstopMakeImage` |
| `src/extended/lowpower/runstop/prt_runstop.c` | 338-352 | `OsRunstopSystemWakeup` |
| `src/extended/lowpower/arch/armv8/prt_lowpower_context_asm.S` | 38-84 / 92-144 | Save / Restore |
| `src/arch/cpu/armv8/common/hwi/prt_hwi.c` | 159 | `OsLowpowerIntWakeupHookCall(hwiNum)` |
| `demos/sd3403/bsp/sd3403/cpu_config.h` | 22-23 | `WOW_IMG_ADDR`/`WOW_IMG_SIZE` |
| `demos/sd3403/bsp/mmu.c` | 48-52 | WOW MMU 映射 |
| `testsuites/lowpower-test/lowpower_test.c` | 151-250 | runstop + readback 用例 |

---

## 11. 时序图与流程详解

> 三条核心时序：**浅睡**（idle 常态）、**Runstop 成像 = 当前深睡验证**（WFI 模拟）、**真深睡**（power-gating + reset-resume，扩展目标）。

### 11.1 浅睡（Light Sleep）idle 流程

**TICKLESS 关（默认）**：`getSleepTime()=0`，`OsLowpowerProcess` 在 runstop 桥+频点后**早返回**（不进 managed 路径、不加锁、不打印），wfi 交 `g_taskCoreSleep`。

```
 IdleTask        OsLowpowerProcess     g_taskCoreSleep      HWI(tick)
    │                   │                    │                  │
    │──idle hook───────>│                    │                  │
    │                   │ runstop flag? no   │                  │
    │                   │ OsChangeFreq? no   │                  │
    │                   │ (无 TICKLESS→早返回)│                  │
    │──coreSleep()──────────────────────────>│                  │
    │                   │        [wfi]       │                  │
    │                   │                    │         tick IRQ │
    │<──────────────────│<─────wfi wakes────────────────────────│
    │  (循环回到 idle)  │                    │                  │
```

**TICKLESS 开**：`getSleepTime()` 返回真实预算，>minSleepTicks 走 managed light-sleep（`OsTicklessOpen` 重装 one-shot + `enterLightSleep=wfi`），≤minSleepTicks 走 work-mode（`wfi` + "work mode" 打印，近事件时命中）。

要点：浅睡 = 单次 `wfi`，被任意中断唤醒，无成像、无 tick 补偿、无 `suspendResumeSem` 握手。对应 LiteOS `OsLightSleepDefault = wfi()`。

### 11.2 Runstop 成像（WFI 模拟深睡 = 当前验证模型）

调用方（测试任务）经 `OsRunstopMakeImage` 阻塞在 `suspendResumeSem`；idle 钩子看到 `OS_STORE_SYSTEM` 后经 `OsWowWriteFlashTask` 执行 `OsRunstopSuspend` 成像分支；WFI 循环被硬件 tick 逐 tick 唤醒；完成后 post sem 唤醒调用方。

```
 TestTask    OsLowpowerProcess   OsWowWriteFlashTask   OsRunstopSuspend   StorageOps(WOW)    HWI(tick)
    │              │                    │                    │                 │               │
    │─MakeImage()  │                    │                    │                 │               │
    │ resumeFlag=RUNSTOP                │                    │                 │               │
    │ flag[0]=STORE│                    │                    │                 │               │
    │─SemPend(suspend) [block]          │                    │                 │               │
    │              │<─idle hook─────────│                    │                 │               │
    │              │─flag==STORE? yes   │                    │                 │               │
    │              │   SemPost(writeFlashSem)                │                 │               │
    │              │                    │<─wake──────────────│                 │               │
    │              │                    │─OsRunstopSuspend──>│                 │               │
    │              │                    │                    │ TaskLock+IntLock│              │
    │              │                    │                    │ SaveRegister()  │               │
    │              │                    │                    │ IsImageResume? yes              │
    │              │                    │                    │─saveImage()────>│ .data/.bss/SR/tick → 0x45200000
    │              │                    │                    │<──────OK────────│               │
    │              │                    │                    │ setWakeUpTimer  │               │
    │              │                    │                    │─enterDeepSleep() (WFI loop ×30) │
    │              │                    │                    │     [wfi]       │      tick IRQ │
    │              │                    │                    │<─wfi wakes (×30)───────────────│
    │              │                    │                    │ withdrawWakeUpTimer              │
    │              │                    │                    │ tickCompensate (elapsed≈30 → 0)  │
    │              │                    │                    │ result=FLASH; postSem=true       │
    │              │                    │                    │ TaskUnlock+IntRestore            │
    │              │                    │                    │─SemPost(suspend)──────────────> │
    │<─SemPend wakes                     │                    │                 │               │
    │ ret=FLASH_IMG_SUCCESS              │                    │                 │               │
    │ resumeFlag=COLD_RESET              │                    │                 │               │
```

要点：
- **Linux 无感**：全程单 mica instance Running，无 stop/start、无 reset、无 PSCI。
- **镜像真实**：`saveImage` 存的是 live `.data/.bss/SR/tick`，readback 逐项比对一致。
- **深睡 = WFI 模拟**：`enterDeepSleep` 默认实现为 WFI 循环（LiteOS `OsEnterDeepSleepDefault = wfi()`）；RAM 保留，故 **不调** `OsLowpowerRestoreRegister`，inline 续跑。
- **tick 补偿 ≈ 0**：WFI 不抑制 tick，挂起期间 tick 自然前进 `elapsed≈sleepTicks`，`sleepTicks-elapsed≈0`。

### 11.3 真深睡（power-gating + reset-resume = 扩展目标）

挂起侧：`enterDeepSleep` 做真掉电（不 inline 返回）；恢复侧：上电复位 → reset vector → `OsLowpowerWakeupFromReset` → `OsRunstopSystemWakeup`（`restoreImage` 回填 RAM + `OsLowpowerRestoreRegister` eret 回到 `OsRunstopSuspend` 的 post-resume 段）。

```
 [挂起 pass]  TestTask … OsRunstopSuspend (按 LiteOS cold-check 模式重构后)
    resumeFlag = COLD_RESET            # 挂起前置 COLD（判别用）
    OsLowpowerSaveRegister()           # 保存 PC = 本句之后
    if (resumeFlag == COLD_RESET):     # cold 首次 pass = 真
        resumeFlag = RUNSTOP_RESET
        saveImage()  ─────────────────> flash/非易失后端
        setWakeUpTimer(RTC)
        enterDeepSleep() ──> [掉电, 不 inline 返回]
    # ↑ reset-resume 后 eret 落到这里；resumeFlag 已被 restoreImage 重载为 RUNSTOP_RESET(≠COLD) → 跳过 if 体
    withdrawWakeUpTimer()
    tickCompensate()
    result = FLASH_IMG_SUCCESS; postSem = true
    SemPost(suspendResumeSem)          # 唤醒 MakeImage 调用方

 [恢复 pass]  上电复位
    ResetVector → 早期 boot → OsLowpowerWakeupFromReset()
        if (resumeFlag == RUNSTOP_RESET):
            OsRunstopSystemWakeup():
                restoreImage()         # 从 flash 回填 .data/.bss（resumeFlag 随之重载为 RUNSTOP_RESET）
                # 注意：此处不清 resumeFlag（否则 eret 后 cold-check 误重入 if 体）
                otherCoreResume = 1
                OsLowpowerRestoreRegister()   # eret → OsRunstopSuspend 中 SaveRegister 之后
```

要点：
- **判别位 `g_lowpowerResumeFromImg`** 是 cold/resume 的开关：挂起前置 `COLD_RESET`、if 体内改 `RUNSTOP_RESET`；恢复后 `restoreImage` 把它重载为 `RUNSTOP_RESET`，故 eret 后 `if (==COLD)` 为假，跳过 if 体直奔 post-resume 段。
- **`OsRunstopSystemWakeup` 不得在 `RestoreRegister` 前清 `resumeFlag`**（当前实现是清的，仅适用 WFI-inline；真深睡需改）。
- `OsLowpowerRestoreRegister` 经 `SPSR_EL1/ELR_EL1` + `eret` 回到 `OsLowpowerSaveRegister` 调用点之后。

---

## 12. 深睡/浅睡流程对比

| 维度 | 浅睡（Light Sleep） | 深睡-当前（WFI 模拟） | 深睡-扩展（power-gating） |
|---|---|---|---|
| 触发 | idle 钩子常态 | `OsRunstopMakeImage` | `OsRunstopMakeImage`（或 PowerMgr 选 DEEP_SLEEP） |
| 入口 op | `enterLightSleep` | `enterDeepSleep`（默认 WFI 循环） | `enterDeepSleep`（客户真掉电） |
| RAM 是否保留 | 是 | 是 | **否**（需 `restoreImage` 回填） |
| SaveRegister | 否（idle 不存） | 是（练手，未用） | 是（复位恢复必需） |
| RestoreRegister | 否 | 否（inline 续跑） | **是**（eret 回挂起点） |
| tick 补偿 | 无 | ≈0（tick 前进） | 全额（tick 已抑制，`sleepTicks-elapsed≈sleepTicks`） |
| 复位向量 | 不走 | 不走 | **走**（`OsLowpowerWakeupFromReset`） |
| Linux 可见性 | 不可见 | 不可见 | 取决于客户实现（若 mica 感知则非纯低功耗） |
| LiteOS 对应 | `OsLightSleepDefault` | `OsEnterDeepSleepDefault` | `OsEnterDeepSleepMainCore` + `OsDeepSleepResume` |

> 当前 sd3403 活跃的是「浅睡」与「深睡-WFI 模拟」。「深睡-扩展」为目标态，代码路径（`OsRunstopSystemWakeup`/`OsLowpowerRestoreRegister`/`restoreImage`）已预留但未在板上验证。

---

## 13. 深睡扩展：需要做什么

### 13.1 客户需提供的实现

1. **`RunstopLowpowerOps`**（`OsRunstopLowpowerOpsReg` 注册）：
   - `enterDeepSleep`：真掉电/门控（**不是** wfi），配置 SoC 进低功耗态、切外设时钟等；不 inline 返回。
   - `setWakeUpTimer(ticks)`：配真唤醒源（RTC/定时器），掉电后能触发上电。
   - `withdrawWakeUpTimer`：唤醒后撤唤醒源。
2. **`RunstopStorageOps`**（`OsRunstopStorageOpsRegister` 注册）：
   - 后端必须**非易失**或**跨掉电保留**（flash / retain-RAM）。当前 WOW `0x45200000`（mcs_mem）仅在 RAM 保留时有效，真掉电需换 flash。
   - `saveImage`：存 `.data/.bss/SR/tick`（含 `g_lowpowerResumeFromImg=RUNSTOP_RESET`）。
   - `validateImage`：复位恢复前校验 magic（`RUNSTOP_MAGIC`）。
   - `restoreImage`：从后端回填 `.data/.bss`（含 `resumeFlag`，使其重载为 `RUNSTOP_RESET`）。
3. **reset vector / 早期 boot**：上电复位后、正常 init 之前，调 `OsLowpowerWakeupFromReset()`。它依 `g_lowpowerResumeFromImg` 分发到 `OsRunstopSystemWakeup`（RUNSTOP）或 `OsDeepSleepResume`（DEEP_SLEEP）。

### 13.2 `OsRunstopSuspend` 需重构为 cold-check 模式

当前 `OsRunstopSuspend`（`prt_runstop.c:202`）为 **WFI-inline 结构**（`if (IsImageResume())` 单分支，无复位重入判别）。真深睡需改成 LiteOS `OsEnterDeepSleepMainCore` 的 cold-check 模式：

```c
void OsRunstopSuspend(void)
{
    ... TaskLock + IntLock ...
    g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;   /* 挂起前置 COLD（判别位）*/
    OsLowpowerSaveRegister();                         /* PC = 本句之后；reset-resume eret 回这里 */
    if (g_lowpowerResumeFromImg == LOWPOWER_COLD_RESET) {   /* cold 首次 pass */
        g_lowpowerResumeFromImg = LOWPOWER_RUNSTOP_RESET;
        saveImage(...);          /* 存镜像（含 resumeFlag=RUNSTOP_RESET）*/
        setWakeUpTimer();
        enterDeepSleep();        /* 真掉电，不 inline 返回 */
        /* rollback(); */        /* 仅 wfi-fail 回滚 */
    }
    /* reset-resume eret 落点：resumeFlag 已被 restoreImage 重载为 RUNSTOP_RESET(≠COLD)，跳过 if 体 */
    withdrawWakeUpTimer();
    tickCompensate();            /* sleepTicks - elapsed(≈0) = sleepTicks 全额补偿 */
    result = FLASH_IMG_SUCCESS; postSem = true;
    ... TaskUnlock + IntRestore ...
    if (postSem) SemPost(g_suspendResumeSem);
    g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;   /* post-resume 完成后再清 */
}
```

### 13.3 `OsRunstopSystemWakeup` 需调整标志清除时机

当前实现（`prt_runstop.c:338`）在 `RestoreRegister` **前**清 `g_lowpowerResumeFromImg = COLD_RESET`——这只适用 WFI-inline。真深睡需：**不在 `RestoreRegister` 前清**（否则 eret 后 cold-check 误重入 if 体，重复 save+sleep）；改到 `OsRunstopSuspend` 的 post-resume 段完成后再清（见 13.2 末行）。

### 13.4 备选：走 PowerMgr 深睡路径

若不想改 `OsRunstopSuspend`，可启用 `CONFIG_OS_OPTION_DEEPSLEEP=y`，用 `prt_lowpower_impl.c` 中已具备 cold-check 结构的 `OsEnterDeepSleepMainCore`（idle 选 DEEP_SLEEP 模式时触发），其复位恢复走 `OsDeepSleepResume` → `OsLowpowerRestoreRegister`。客户实现 `PowerMgrDeepSleepOps`（`suspendDevice/resumeDevice/resumeFromReset/...`）即可。该路径已对齐 LiteOS，但与 runstop 成像是两条独立路径。

---

## 14. 当前验证方式

### 14.1 构建与上板

```bash
# 容器内构建
cd /home/uniproton/UniProton/demos/sd3403/build
sh build_app.sh lowpower          # 产物 lowpower.elf / lowpower.bin

# 上板（宿主机）
python3 /tmp/opencode/deploy_lowpower.py
#  → scp lowpower.bin 到板子
#  → mica 启动 instance（单实例）
#  → 抓串口日志到 demos/sd3403/build/test-results.txt
#  → 清理 instance
```

### 14.2 测试入口

`demos/sd3403/apps/openamp/main.c` 的 `LOWPOWER_TESTCASE` 下调 `lowpower_test()`（`testsuites/lowpower-test/lowpower_test.c`）。

### 14.3 用例覆盖与通过判据

| 用例 | 判据 | 最近板上结果 |
|---|---|---|
| PowerMgr 投票 | `vote 0→2→1` | `2`/`1` ✓ |
| Tickless（关） | 早返回+`g_taskCoreSleep=wfi`、0 work-mode | `option disabled`、0 work-mode ✓ |
| Tickless（开） | managed sleep、`OsSleepTicksGet`=budget | `OsSleepTicksGet=70`、one-shot `+20`、loop `10 10…` ✓ |
| Runstop 成像 | `MakeImage` 返 `FLASH_IMG_SUCCESS`，挂起 ≈30 tick | `suspended ~31 ticks` ✓ |
| saveImage 回调 | `g_saveImageCalled==true` | ✓ |
| validateImage | magic 校验 OK | ✓ |
| Readback `.bss` marker | `0xa5a55a5a == live` | `0xa5a55a5a == 0xa5a55a5a` ✓ |
| Readback `.data` var | `0x123456789abcdef0 == live` | ✓ |
| Readback tick | saved < current | `saved=641 current=672` ✓ |

通过：`[LOWPOWER] RESULT: PASS`（0 failures）。

### 14.4 验证模型边界（已验证 / 未验证）

| 项 | 状态 |
|---|---|
| PowerMgr 投票模型（vote begin/end/count） | ✅ 已验证 |
| Runstop 真实镜像保存（`.data/.bss/SR/tick`） | ✅ 已验证（readback 比对） |
| WFI 模拟深睡 + 硬件 tick 唤醒 + inline 续跑 | ✅ 已验证 |
| Linux 透明（单 instance，无 mica 重启） | ✅ 已验证 |
| tick 补偿路径 | ✅ 已验证（≈0 场景） |
| 真深睡（power-gating + reset-resume + `RestoreRegister`） | ❌ 未验证（预留，见 §13） |
| Tickless（`CONFIG_OS_OPTION_TICKLESS`） | ✅ 已验证（关：0 work-mode；开：one-shot `+20`/loop `10 10…`/runstop PASS，见 §15） |
| DEEPSLEEP 路径（`OsEnterDeepSleepMainCore`） | ❌ 未验证（宏关闭） |

---

## 15. Tickless 适配详解（non-SMP 移植）

> 原生 UniProton tickless 仅在 `CONFIG_OS_OPTION_SMP=y` 路径实现/编过，**non-SMP 路径未实现且未编过**。本节记录按 LiteOS 流程移植到 non-SMP 的全部改动、补偿模型推导（为什么 tick-IRQ 补 `sleepTicks-1`）、swtmr 扫描从 cursor 追赶改为 expectedTick 绝对扫描的原因，以及关/开对比与板上验证结果。

### 15.1 原生 non-SMP gap

非 SMP 下以下点缺/错：

- `PRT_TickISR`（`src/core/kernel/tick/prt_tick.c`）未设 tick-IRQ 标志，idle 的 `OsTicklessOpen` 无法判断"上一 tick 是否真 fire"，one-shot 不会重装。
- `PRT_LowpowerInit` 未调 `PRT_TicklessEnable`（LiteOS `los_init` 调 `LOS_TicklessEnable`）。
- `OsSwTmrScan`（`src/core/kernel/timer/swtmr/prt_swtmr.c`）non-SMP 版为 **cursor 推进一格** 的时间轮；tickless 补偿 `g_uniTicks += N` 后 cursor 只 +1、落后 N-1，swtmr 延迟到期（曾实测 one-shot `+47`、loop 间隔 `11 11…`）。
- `prt_tickless.c` 编译错误（全局变量缺 `static`、`OS_TICK_PER_SECOND` 宏在适配层不可用、缺 include）。
- `prt_swtmr_minor.c` non-SMP 路径引用 SMP-only `coreID`，编译断。

### 15.2 改动清单（全部 `#if defined(OS_OPTION_TICKLESS)` 门控，关时零影响）

| 文件 | 改动 | LiteOS 对应 |
|---|---|---|
| `src/core/kernel/tick/prt_tick.c` `PRT_TickISR` | `OsTickIrqFlagSet(OsTicklessFlagGet())` | `OsTickHandler` 末尾 |
| `src/extended/lowpower/prt_lowpower.c` `PRT_LowpowerInit` | `PRT_TicklessEnable()` + `g_taskCoreSleep=wfi` | `los_init` |
| `src/core/kernel/timer/swtmr/prt_swtmr_internal.h` | `OS_SWTMR_SORT_LINK(swtmr)` 双模宏（SMP→coreID sortlink，non-SMP→全局） | — |
| `src/core/kernel/timer/swtmr/prt_swtmr_minor.c` | 用 `OS_SWTMR_SORT_LINK` 替 SMP-only `coreID` | — |
| `src/core/kernel/timer/swtmr/prt_swtmr.c` `OsSwTmrScan` | non-SMP tickless 分支：**expectedTick 绝对扫描**（§15.4） | `OsSwtmrScan`（SMP 版按 expectedTick） |
| `src/core/kernel/task/amp/prt_amp_task.c` | 新增 `OsAmpTskDlyNearestTickGet`（读 task delay 有序链头 `expirationTick`） | SMP `OsTskDlyNearestTickGet` |
| `src/core/kernel/task/amp/prt_amp_task_init.c` | `OsTskAMPInit` `#if TICKLESS` 注册 `g_getTskDlyNearestTick`（task delay 进 budget） | SMP `OsTskAMPInit` 注册 |
| `src/extended/lowpower/tickless/prt_tickless.c` | `static` 存储类；`OsSysGetTickPerSecond()` 运行时；include；新增 `PRT_TicklessSetTickIrqNum`；`OsTicklessUpdate` 双分支（§15.3） | `OsTicklessUpdate` |
| `testsuites/lowpower-test/lowpower_test.c` | `LowpowerTestTickless` 调 `PRT_TicklessSetTickIrqNum(TEST_CLK_INT)` | — |

### 15.3 补偿模型推导（为什么 tick-IRQ 补 `sleepTicks-1`）

**UniProton non-SMP tick 路径把一次 tick-IRQ fire 的账目拆成两段**（tick 路径已厘清）：

```
CNTP one-shot fire          (物理已过去 budget 个 tick)
  └─ HWI: OsHwiDispatchHandle(hwiNum=30)
        ├─ OsLowpowerIntWakeupHookCall(30)   ──【第一段】OsTicklessUpdate（我控制）
        │     └─ OsTicklessUpdate(30)
        └─ OsHwiHookDispatcher(30) → BSP TimerIsr → PRT_TickISR
              └─ rq->tickNoRespondCnt++        (deferred，不立即扫任务/软定时器)
  └─ IRQ 尾部: while (tickNoRespondCnt>0) { g_tickDispatcher(); tickNoRespondCnt--; }
        └─ OsTickDispatcher
              ├─ g_uniTicks++                  ──【第二段】固定 +1（每次 PRT_TickISR 对应一次）
              ├─ OS_TASK_SCAN()
              └─ OS_SWTMR_SCAN()
```

**约束**：CNTP fire 时物理已过去 `budget` 个 tick，故两段补偿之和必须 = `budget`。

- 第二段（`OsTickDispatcher`）固定 `+1`：因为 `PRT_TickISR` 跑了一次，`tickNoRespondCnt` 增了 1，IRQ 尾部 `g_tickDispatcher()` 被调用一次、`g_uniTicks += 1`（源码依据：`src/core/kernel/irq/prt_irq_minor.c:99`）。
- 因此第一段（`OsTicklessUpdate`）必须补 `budget - 1`，两段相加才等于真实经过的 `budget`。

`OsTicklessUpdate` 的 tick-IRQ 分支（`prt_tickless.c`）：

```c
} else if (irqNum == g_tickIrqNum) {
    OsSysTimeUpdate(sleepTicks - 1);   // sleepTicks == budget
}
```

其中 `OsSysTimeUpdate(N)` → `PRT_TickCountUpdate(OS_TYPE_LIGHT_SLEEP, N)` → `g_uniTicks += N`（**全 N**，不减 1）。

**对比 LiteOS**：LiteOS 的 `OsSysTimeUpdate(N)` 内部是 `g_tickCount += N - 1`（减 1），其 tick-IRQ 分支传 `sleepTicks` → 实际 `+= sleepTicks - 1`，再加 tick ISR 的 `+1` = `sleepTicks`。UniProton 的 `PRT_TickCountUpdate` 是 `+= N`（不减），所以传 `sleepTicks - 1`，**两者等价**。

> ⚠️ 这个 `-1` 不是测出来的硬凑数，是 deferred-tick 架构（`tickNoRespondCnt` + `OsTickDispatcher +1`）的必然结果：fire 一次 → `tickNoRespondCnt+1` → 尾部 `OsTickDispatcher` 一次 → `g_uniTicks+1`。

#### 15.3.1 早期唤醒分支（非 tick IRQ，无 deferred +1）

非 tick IRQ 在 budget 到期前唤醒：`PRT_TickISR` **没跑**，`tickNoRespondCnt` 不增，IRQ 尾部无 `OsTickDispatcher` → **没有 `+1`**。补偿 = 实际 elapsed cycle 折算的 tick 数（**无 -1**），并把定时器重新装到剩余量：

```c
cycles   = CNTP_TVAL_EL0                         // 剩余 cycle（未 wrap）
elapsed  = sleepTicks*cyclesPerTick - cycles
ticks    = elapsed / cyclesPerTick
OsSysTimeUpdate(ticks)                            // += ticks（无 -1，因无 deferred +1）
OsTickTimerReload(cyclesPerTick - elapsed % cyclesPerTick)   // 重装剩余，继续睡
```

#### 15.3.2 tick IRQ 号注入（对标 LiteOS `OS_TICK_INT_NUM`）

适配层位于 `src/extended/`（内核库），拿不到 BSP 的 `TEST_CLK_INT`(30)。故新增 `PRT_TicklessSetTickIrqNum(irqNum)`，由 BSP/应用在初始化时注入。测试 `LowpowerTestTickless`（`lowpower_test.c:158`）调 `PRT_TicklessSetTickIrqNum(TEST_CLK_INT)`。LiteOS 用内核级 `OS_TICK_INT_NUM` 宏，UniProton 无此内核级宏，故走 setter。

### 15.4 swtmr 扫描：cursor 追赶 → expectedTick 绝对扫描

**问题（cursor 追赶）**：原 non-SMP `OsSwTmrScan` 是时间轮 cursor 推进一格（`cursor=(cursor+1)%64`）。tickless 下 wake 时 `g_uniTicks` 跳了 N，但 cursor 只能 +1、落后 N-1：

- one-shot swtmr（20 tick）：cursor 需 20 次 `OsTickDispatcher` 才追上，swtmr 延迟到期（曾实测 `+47`）。
- loop swtmr（10 tick）：每周期 cursor 半追赶，loop 定时器触发时序错乱、间隔漂移（曾实测 `11 11 11…`）。

**修复（expectedTick 绝对扫描，`prt_swtmr.c`）**：tickless 下 `OsSwTmrScan` 改为扫 `g_swtmrCbArray`，`expectedTick <= g_uniTicks` 即到期（与 budget 查询 `OsSwtmrNearestTickGet` 同款）：

```c
for (idx = 0; idx < g_swTmrMaxNum; idx++) {
    swtmr = g_swtmrCbArray + idx;
    if (state == FREE || CREATED)  continue;
    if (swtmr->expectedTick > g_uniTicks) continue;   // 未到期
    swtmr->state = OS_TIMER_EXPIRED;
    // 从 wheel 槽链表摘除（tickless 下 rollNum 不用于到期判定）
    摘链 (next/prev = NULL);
    handler(...);                 // 回调（解锁执行，可能重入 swtmr API）
    OsSwtmrProc(swtmr);           // loop→重算 expectedTick 重启；单次→置 CREATED；删除
}
```

理由：tickless 模型是"睡到最近事件再唤醒"，wake 时 `g_uniTicks` 已在该事件点，应只触发"已到期"的（通常 0~1 个），**O(待到期)** 而非 O(wheel) cursor 追赶。`expectedTick` 在 `OsSwTmrStart` 时已算好（`#if TICKLESS||SMP`，`prt_swtmr.c:334`：`swtmr->expectedTick = interval + g_uniTicks`），扫描直接复用。loop 定时器在 `OsSwtmrProc` 重启时基于"当前 `g_uniTicks`"重算 `expectedTick`，故间隔稳定为 `10 10 10…`。

**非 tickless（默认）** 保留原 cursor 时间轮 `OsSwTmrScanOneSlot`，零回归。

### 15.5 关 vs 开对比

| 维度 | TICKLESS 关（默认） | TICKLESS 开 |
|---|---|---|
| idle 行为 | `OsLowpowerProcess` 早返回 + `g_taskCoreSleep=wfi`（无锁、无打印） | managed：`getSleepTime()`=budget，>minSleepTicks→重装 one-shot+wfi |
| swtmr 扫描 | cursor 时间轮（`OsSwTmrScanOneSlot`） | expectedTick 绝对扫描（`OsSwTmrScan` tickless 分支） |
| tick 补偿 | 无（tick 正常前进） | tick-IRQ 补 `budget-1` + deferred `+1` = `budget` |
| work-mode 打印 | 0（设计修复，§6.1） | 近事件 budget<minSleepTicks 时合法打印（LiteOS 行为） |

### 15.6 Work-mode 刷屏根因与修复

现象：TICKLESS 开时 `[PM] Application is running in work mode` 大量刷屏。LiteOS 同款 `OsLowpowerProcess` + 同条 `PRINT_WARN` 应不刷，定位到两处根因：

**根因 1（适配层 bug）：`OsSleepTicksGet` 的 idle 分支返回 0**。`PRT_TickLessCountGet`（`prt_tick.c:177`）在所有核最近事件均为 `FOREVER`（纯 idle）时置 `*minTicks=OS_MAX_U32` 并返回 FAIL；原 `OsSleepTicksGet`（`prt_tickless.c`）在 FAIL 时 `return 0` → `getSleepTime()=0 <= minSleepTicks` → **每次 idle 迭代都 work-mode**。LiteOS 的 `OsSleepTicksGet`（取 task/swtmr sortlink 最近到期，空链返回大值）idle 时返回大值进 managed sleep。修复：FAIL/OK 均返回 `minTicks`（idle 时即 `OS_MAX_U32`）：

```c
(void)PRT_TickLessCountGet(&minTicks, &coreId);
return minTicks;   /* idle -> OS_MAX_U32 -> managed light sleep, no work-mode */
```

**根因 2（测试环境）：openamp demo 的 1000ms(100-tick) LOOP swtmr**。`demos/sd3403/apps/openamp/main.c` 的 `TimerTestStart()` 在 `PRT_AppInit` 无条件建一个 `interval=1000`ms LOOP 定时器（100Hz 下 = 100 tick），永久运行 → budget 恒 ≤ `minSleepTicks(100)` → work-mode 刷屏（probe 实测 idle 时 `OsSleepTicksGet` 恒为 100）。LiteOS 裸机 demo 无此定时器。修复：`TimerTestStart()` 用 `#if !defined(LOWPOWER_TESTCASE)` 门控（lowpower 测试自带 swtmr）。

**根因 3（内核缺陷）：非 SMP 未注册 `g_getTskDlyNearestTick` → task delay 不进 tickless budget**。`g_getTskDlyNearestTick` 只在 SMP init（`prt_smp_task_init.c:213`）注册，AMP（非 SMP）完全不注册。后果：`PRT_TickDelay(N)` 的 task 不在 budget，idle managed sleep 只按下一个 swtmr/cpup 事件（或 `maxSleepCount=10000`）补偿，远超 N → task delay 被推迟、`g_uniTicks` 跳跃虚高（实测同样测试流程，ON 时 readback tick=34770，OFF 时=811，差 ~40×）。LiteOS 的 `OsSleepTicksGet` 直接读 task sortlink（含 delay），无此问题。修复：`prt_amp_task.c` 新增 `OsAmpTskDlyNearestTickGet`（读 AMP 全局 task delay 有序链 `g_tskSortedDelay.tskList` 头节点 `expirationTick`，绝对值，与 `OsSwtmrNearestTickGet` 同语义），`OsTskAMPInit` `#if TICKLESS` 注册到 `g_getTskDlyNearestTick`。修复后 ON 时 readback tick=812，与 OFF 的 811 一致（虚高消除）。

修复后：纯 idle（`test end` 之后）work-mode = **0**（managed light sleep）；仅测试的紧 swtmr 阶段（20/10-tick 定时器）出现合法 work-mode（budget 真的 <100，与 LiteOS 同款定时器下行为一致）。

### 15.7 板上验证结果

| 配置 | one-shot swtmr | loop swtmr | runstop | readback tick | work-mode | RESULT |
|---|---|---|---|---|---|---|
| TICKLESS 开 | `+20` ✓ | `10 10 10 10 10` ✓ | PASS | ~812（与关一致，虚高已修） | idle=0（§15.6 修复后）；紧 swtmr 期 ~35（合法） | **PASS** |
| TICKLESS 关 | `+20` ✓ | `10 10…`（cursor 时间轮） | PASS | ~811 | 0（早返回+`g_taskCoreSleep=wfi`） | **PASS** |

通过判定：`[LOWPOWER] RESULT: PASS`（0 failures）。验证脚本/日志：`/tmp/opencode/deploy_lowpower.py`、`demos/sd3403/build/test-results.txt`。

---


# 内存管理介绍

内存管理主要工作是动态的划分并管理用户分配好的内存区间。当程序某一部分需要使用内存，可以通过操作系统的内存申请函数索取指定大小内存块，一旦使用完毕，通过内存释放函数归还所占用内存，使之可以重复使用。在系统运行过程中，内存管理模块通过对内存的申请/释放操作管理用户和OS对内存的使用，使内存的利用率和使用效率达到最优。

## 内存基本概念

### 内存块 slice
用户申请到的一片连续内存空间，是内存管理的最小单元。

### 算法 arithmetic
内存管理的一种策略，如FSC。

## 内存算法

UniProton 内存管理支持在编译期通过 defconfig 选择底层分配算法。所有算法对外提供统一的**标准接口**，上层（`PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign`，以及内核各模块直接调用的 `OsMemAlloc`/`OsMemAllocAlign`）无需感知具体算法，由公共分派层根据宏控选用。

### 算法选择与公共分派

- **defconfig 选择**：`CONFIG_OS_MEM_ARITH_TLSF=y` 选用 TLSF；缺省（不设置）用 FSC。
- **公共初始化 `OsMemInit(addr, size)`**（`src/mem/prt_mem.c`）：按宏分派到对应算法自己的初始化实现。
  - FSC：`OsFscMemInit`
  - TLSF：`OsTlsfMemInit`
- 各算法的 init 负责初始化内存池，并填充 `g_memArithAPI` 分发表（`alloc`/`allocAlign`/`free` 三个函数指针）。
- `prt_mem.c` 中的 `PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign` 经 `g_memArithAPI` 表分派到当前算法。
- 各板级 `OsMemConfigReg` 只调用公共 `OsMemInit`，不直接指定算法；切换算法仅需改 defconfig，无需改板级代码。

```
OsMemConfigReg ──> OsMemInit ──┬─ OS_MEM_ARITH_TLSF: OsTlsfMemInit ──> 填 g_memArithAPI
                               └─ 否则:              OsFscMemInit  ──> 填 g_memArithAPI

PRT_MemAlloc/Free/AllocAlign ──> g_memArithAPI.{alloc,free,allocAlign}
内核模块 ──> OsMemAlloc/OsMemAllocAlign（算法无关共享符号，编译期选定算法实现）
```

### FSC 算法（缺省）

- **FSC**（Fixed-Size Chunk，定长块）：按块大小索引，位图（`g_fscMemBitMap`）+ 链表快速查找，O(1) 分配。
- 源码：`src/mem/fsc/prt_fscmem.c`，实现 `OsFscMemInit`/`OsFscMemFree` 及共享的 `OsMemAlloc`/`OsMemAllocAlign`。
- 节点尾部带 `OS_FSC_MEM_TAIL_MAGIC` 溢出检测（这是 FSC 专有机制，TLSF 无此机制）。

### TLSF 算法

- **TLSF**（Two-Level Segregated Fit，两级分隔适配）：O(1) 的通用内存分配算法，移植自 LiteOS-M 并已去除全部 `LOS_` 前缀符号，作为 UniProton 原生算法维护。
- 源码目录 `src/mem/tlsf/`：

| 文件 | 说明 |
|---|---|
| `prt_tlsf_core.c` / `prt_tlsf_core.h` | TLSF 算法核心，提供 `OsTlsfInit`/`OsTlsfAlloc`/`OsTlsfAllocAlign`/`OsTlsfFree`/`OsTlsfRealloc`/`OsTlsfInfoGet` 等 |
| `prt_tlsf_list.h` | 算法自用的双向链表（`OsTlsfList*`/`OS_TLSF_DL_LIST*`） |
| `prt_tlsf_compat.h` | 类型/打印/hook/任务桩等适配层，直接对接 UniProton 原生 `PRT_` 接口（如 `PRT_HwiLock`/`PRT_Printf`） |
| `prt_tlsf_config.h` | 算法配置宏（`TLSF_CFG_*`），裁剪 leakcheck/integrity/task-mem/lms/lmk 等可选项 |
| `prt_tlsfmem.c` | 胶水层：`OsTlsfMemInit`（注册到 `g_memArithAPI`）、`OsMemAlloc`/`OsMemAllocAlign`/`OsTlsfMemFree`（包装算法接口 + 维护统计量） |

- **与 FSC 行为对齐**：`size==0` 返回 NULL；对齐 `boundary` 小于指针宽度时钳到 `sizeof(VOID*)`；维护 `g_memUsage`/`g_memPeakUsage`/`g_memTotalSize`/`g_memStartAddr` 统计量供 memInfo 使用。
- **暂不支持 expand**（`OS_MEM_EXPAND_ENABLE=0`），即不动态扩展内存池。

### TLSF 配置宏

TLSF 算法的可选项由 `prt_tlsf_config.h` 的 `TLSF_CFG_*` 宏控制（`prt_tlsf_core.c` 中以 `#if` 守护）。移植时已为所有宏设置安全默认值。下面按「能否开启」分类说明。

#### 当前无法开启（依赖的 LiteOS 子系统未移植到 UniProton）

开启下列任一宏会引入当前不存在的依赖，**必须保持关闭**：

- **`OS_MEM_EXPAND_ENABLE`**（在 `prt_tlsf_core.c` 中硬编码为 0）
  - 功能：内存池耗尽时动态向系统申请新页扩展池容量。
  - 缺失依赖：页分配器 `OsTlsfPhysPagesAlloc`、`PAGE_SHIFT`、`OsTryShrinkMemory`。

- **`TLSF_CFG_KERNEL_LMS`**（默认不定义，`#ifdef` 判断）
  - 功能：LMS（Lite Memory Sanitizer）内存越界 / use-after-free 检测。
  - 缺失依赖：LMS 模块（`los_lms_pri.h`）。

- **`TLSF_CFG_KERNEL_LMK`**（默认 0）
  - 功能：LMK，内存耗尽时按策略 kill 任务以回收内存。
  - 缺失依赖：`OsTlsfLmkTasksKill`。

- **`TLSF_CFG_MEM_LEAKCHECK`**（默认 0）
  - 功能：内存泄漏检查，为每个分配节点记录调用栈（返回地址）。
  - 缺失依赖：回溯接口 `OsBackTraceHookCall`；配套宏 `TLSF_CFG_MEM_RECORD_LR_CNT` / `TLSF_CFG_MEM_OMIT_LR_CNT` 同样不可开。

- **`TLSF_CFG_PLATFORM_EXC`**（默认 0）
  - 功能：与异常子系统联动，异常时 dump 内存池信息。
  - 缺失依赖：LiteOS 异常模块。

#### 当前可以开启（自包含，无外部依赖）

- **`TLSF_CFG_MEM_WATERLINE`**（默认 1）
  - 功能：记录内存使用水位（峰值），供统计查询。

- **`TLSF_CFG_MEM_MUL_POOL`**（默认 1）
  - 功能：支持创建 / 管理多个独立内存池。

- **`TLSF_CFG_MEM_MUL_REGIONS`**（默认 0）
  - 功能：把多段不连续物理内存加入同一内存池（`OsTlsfRegionsAdd`）。

- **`TLSF_CFG_TASK_MEM_USED`**（默认 1）
  - 功能：在节点头记录申请该内存块的任务 ID。
  - 注意：会影响节点头布局；**64 位平台必须为 1**（否则节点头 padding 会导致 `OsTlsfFree` 取真实指针错位）。

- **`TLSF_CFG_MEM_FREE_BY_TASKID`**（默认 0）
  - 功能：按任务 ID 释放该任务占用的全部内存。
  - 约束：当 `TLSF_CFG_TASK_MEM_USED != 1` 且 `TLSF_CFG_BASE_CORE_TSK_LIMIT + 1 > 64` 时编译报错。

- **`TLSF_CFG_BASE_MEM_NODE_INTEGRITY_CHECK`**（默认 0）
  - 功能：节点完整性检查（魔字 / 链表一致性校验）。
  - 注意：可正常编译；但完整性失败 panic 路径中的任务信息查询（`OS_TLSF_TCB_FROM_TID`）目前为桩实现。

- **`TLSF_CFG_KERNEL_PRINTF`**（默认 1）
  - 功能：TLSF 内部打印开关。

- **`TLSF_CFG_BASE_CORE_TSK_LIMIT`**（默认 31）
  - 功能：任务数上限，用于 taskID 相关校验。

- **`TLSF_CFG_SYS_EXTERNAL_HEAP`**（默认 1）
  - 功能：使用外部传入的堆（由 UniProton 的 `OS_MEM_FSC_PT_ADDR/SIZE` 指定），不使用算法自带静态堆数组。

### 目录结构

```
src/mem/
  prt_mem.c              公共分派层（PRT_Mem* + OsMemInit）
  prt_mem_internal.h     TagMemFuncLib 分发表定义
  include/
    prt_mem_external.h      算法无关公共声明（OsMemInit/OsMemAlloc/OsMemAllocAlign）
    prt_fscmem_external.h   FSC 算法声明
    prt_tlsfmem_external.h  TLSF 算法声明
  fsc/                   FSC 算法实现
  tlsf/                  TLSF 算法实现
```

### 新增算法

新增一种内存分配算法（如 bestfit / slab）的步骤：

1. 在 `src/mem/<algo>/` 下实现自己的初始化 `Os<Algo>MemInit` 与释放 `Os<Algo>MemFree`，以及共享名的 `OsMemAlloc`/`OsMemAllocAlign`，并在 init 中填充 `g_memArithAPI` 分发表。
2. 在 `src/mem/Kconfig` 增加 `config OS_MEM_ARITH_<ALGO>`。
3. 在 `src/mem/CMakeLists.txt` 增加对应编译分支。
4. 在 `prt_mem.c` 的 `OsMemInit` 中增加 `#elif defined(OS_MEM_ARITH_<ALGO>)` 分派分支。
5. 板级 defconfig 设置该宏即可切换，无需改动 `OsMemConfigReg`。

## 开发流程
### 步骤一：设置内存管理模块配置项

使用UniProton内存管理模块，需要进行配置项的设置，需要配置的项包括缺省分区首地址、分区大小等。

### 步骤二：使用内存管理模块

当需要使用内存时，需要先创建一个指定内存管理算法的内存分区，通过调用内存申请接口申请合适大小的内存，就可以对申请到的内存进行操作（包括写操作，然后给其他模块传递消息等）；如果是动态内存，当内存使用完，需要对这块内存进行释放，防止发生内存泄漏。

## 测试

### 测试分类原则

由于各算法仅底层实现不同、对外标准接口一致，因此**绝大部分测试用例是算法无关的（common）**，FSC 与 TLSF 两种算法都应通过；只有极少数用例依赖某算法专有机制，归为该算法独有：

| 分类 | 说明 | 运行条件 |
|---|---|---|
| **common** | 经标准接口（`PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign` 或 libc malloc）验证分配/释放/对齐/复用等功能，不依赖任何算法私有布局 | 两种算法都跑 |
| **FSC 专有** | 依赖 FSC 私有机制（如节点尾部 `OS_FSC_MEM_TAIL_MAGIC` 溢出检测） | 仅 FSC 跑 |
| **TLSF 专有** | 依赖 TLSF 私有机制 | 仅 TLSF 跑 |

### 测试 app

| app | 源文件 | 覆盖 | 说明 |
|---|---|---|---|
| `UniPorton_test_posix_malloc_interface` | `runMallocTest.c` + posix malloc 用例 | libc malloc/calloc/realloc/memalign/reallocarray/usable_size 一致性 | common，经 libc→`PRT_Mem` |
| `UniPorton_test_prt_mem_interface` | `runPrtMemTest.c` | `PRT_Mem*` 直接接口 | common，直接验证 `PRT_MemAlloc`/`Free`/`AllocAlign` |

`runPrtMemTest` 当前 10 个 common 用例：

| 用例 | 验证点 |
|---|---|
| prt_mem_001 | 基本分配 + 写入校验 + 释放 |
| prt_mem_002 | `size==0` 返回 NULL |
| prt_mem_003 | 释放后再次分配（块复用） |
| prt_mem_004 | `PRT_MemAllocAlign` 对齐分配 + 地址对齐校验 |
| prt_mem_005 | 多块分配/全释放/合并后大块可用 |
| prt_mem_006 | realloc 扩容/缩容 + 内容保持 |
| prt_mem_007 | 超大 size 申请返回 NULL（OOM 边界） |
| prt_mem_008 | 多种对齐值（4/8/16/32/64）逐一校验 |
| prt_mem_009 | `PRT_MemFree(NULL)` 返回错误码 |
| prt_mem_010 | 交错分配/释放压力，不损坏堆 |

### 算法独有用例

- **`malloc_usable_size_1_1`**（FSC 专有）：校验 `*(U32*)(ptr + usableSize) == OS_FSC_MEM_TAIL_MAGIC`，依赖 FSC 的 tail magic 机制。在 `runMallocTest.h` 中以 `#ifndef OS_MEM_ARITH_TLSF` 守护，TLSF 模式下排除（TLSF 无此机制，为保证算法原有行为不伪造 magic）。

### 验证结果（sd3403，armv8）

两种算法均上板验证，结果如下：

| 算法（defconfig） | prt_mem（common 10） | malloc（一致性） |
|---|---|---|
| TLSF（`CONFIG_OS_MEM_ARITH_TLSF=y`） | 10/10 通过 | 10/10 通过（usable_size 排除） |
| FSC（缺省） | 10/10 通过 | 11/11 通过（含 usable_size） |

结论：common 用例两算法全通过；FSC 专有的 `malloc_usable_size_1_1` 仅在 FSC 模式运行并通过；`OsMemInit` 公共分派的 FSC/TLSF 两个分支均已上板验证。

### 上板运行（sd3403）

构建（容器内）：
```bash
cd demos/sd3403/build
sh build_app.sh UniPorton_test_prt_mem_interface
sh build_app.sh UniPorton_test_posix_malloc_interface
```
切换算法：修改 `build/uniproton_config/config_armv8_sd3403/defconfig` 中 `CONFIG_OS_MEM_ARITH_TLSF` 后重新构建。部署/运行/采集串口日志/清理的生命周期管理按 sd3403 deploy 流程（SCP 传 ELF → mica 配置 CPU3 → 触发 → 采集 → `mica stop`/`rm`）。

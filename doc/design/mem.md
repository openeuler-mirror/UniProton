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

- **defconfig 选择**：`CONFIG_OS_MEM_ARITH_TLSF=y` 选用 TLSF；`CONFIG_OS_MEM_ARITH_BESTFIT_LITTLE=y` 选用 bestfit_little；`CONFIG_OS_MEM_ARITH_BESTFIT=y` 选用 bestfit；缺省（不设置）用 FSC。
- **slab 扩展**：`CONFIG_OS_MEM_SLAB_EXTENTION=y` 在 bestfit_little 和 bestfit 模式下生效，作为小块分配 cache 挂在对应 heap 算法前面，不是独立的 UniProton 内存算法。
- **slab 自动扩展**：`CONFIG_OS_MEM_SLAB_AUTO_EXPANSION_MODE=y` 依赖 `CONFIG_OS_MEM_SLAB_EXTENTION=y`，允许 slab class 在初始 bucket 用完后继续从 backing heap 申请 bucket。
- **公共初始化 `OsMemInit(addr, size)`**（`src/mem/prt_mem.c`）：按宏分派到对应算法自己的初始化实现。
  - FSC：`OsFscMemInit`
  - TLSF：`OsTlsfMemInit`
  - bestfit_little：`OsBestfitLittleMemInit`
  - bestfit：`OsBestfitMemInit`
- 各算法的 init 负责初始化内存池，并填充 `g_memArithAPI` 分发表（`alloc`/`allocAlign`/`free` 三个函数指针）。
- `prt_mem.c` 中的 `PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign` 经 `g_memArithAPI` 表分派到当前算法。
- 各板级 `OsMemConfigReg` 只调用公共 `OsMemInit`，不直接指定算法；切换算法仅需改 defconfig，无需改板级代码。

```
OsMemConfigReg ──> OsMemInit ──┬─ OS_MEM_ARITH_TLSF:           OsTlsfMemInit ──> 填 g_memArithAPI
                               ├─ OS_MEM_ARITH_BESTFIT_LITTLE: OsBestfitLittleMemInit ──> 填 g_memArithAPI
                               ├─ OS_MEM_ARITH_BESTFIT:        OsBestfitMemInit ──> 填 g_memArithAPI
                               └─ 否则:                        OsFscMemInit  ──> 填 g_memArithAPI

PRT_MemAlloc/Free/AllocAlign ──> g_memArithAPI.{alloc,free,allocAlign}
内核模块 ──> OsMemAlloc/OsMemAllocAlign（算法无关共享符号，编译期选定算法实现）
```

### 公共接口契约

所有算法都必须遵守 UniProton 内存管理公共契约：

- 初始化入口固定为 `OsMemInit(addr, size)`，由公共分派层根据 `OS_MEM_ARITH_*` 宏调用具体算法初始化函数。
- 对外接口固定为 `PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign`，内部通过 `g_memArithAPI` 分发表调用当前算法。
- 内核内部共享符号固定为 `OsMemAlloc`/`OsMemAllocAlign`，由当前编译进来的算法提供实现，避免上层模块感知算法差异。
- `size == 0` 返回 `NULL`；分配失败返回 `NULL`；非法释放返回错误码。
- 对齐分配必须返回满足 `boundary` 的地址，且释放路径能还原真实节点头。
- 统计信息统一维护到 `g_memUsage`/`g_memPeakUsage`/`g_memTotalSize`/`g_memStartAddr`，供 memInfo 和测试使用。
- 算法私有入口使用 `PRT_` 前缀；设计文档可以说明来源于 LiteOS，但 UniProton 侧不继续对外暴露 `LOS_Mem*` 接口名。

### FSC 算法（缺省）

- **FSC**（Fixed-Size Chunk，定长块）：UniProton 缺省内存算法，按空闲块大小建立分级链表，用位图快速定位可用链表。
- 源码：`src/mem/fsc/prt_fscmem.c`，实现 `OsFscMemInit`/`OsFscMemFree` 及共享的 `OsMemAlloc`/`OsMemAllocAlign`。

**原理与数据结构**

- 每个内存块使用 `TagFscMemCtrl` 描述大小、前一块大小、前后链表指针等元数据。
- 空闲块按大小映射到 `g_fscMemNodeList[OS_FSC_MEM_LAST_IDX]`。`OS_FSC_MEM_SZ2IDX(size)` 根据最高有效位计算 size class，`OS_FSC_MEM_IDX2BIT(idx)` 映射到 `g_fscMemBitMap`。
- `g_fscMemBitMap` 记录哪些 size class 非空，分配时先按申请大小得到目标 class，再通过位图寻找不小于目标大小的空闲链表。
- 空闲块插入/删除由 `OsFscMemInsert`/`OsFscMemDelete` 完成，链表头和位图同步更新。

**分配流程**

1. `OsMemAlloc` 对申请大小做最小块和地址对齐处理。
2. 根据对齐后的大小计算目标 class，结合 `g_fscMemBitMap` 找到可容纳的空闲块。
3. 若空闲块大于申请大小且剩余空间足够容纳一个最小块，则调用 `OsFscMemSplit` 分裂。
4. 标记节点为 used，返回节点头之后的用户地址。

**释放流程**

1. `OsFscMemFree` 根据用户地址还原 `TagFscMemCtrl` 节点头。
2. 校验 used magic、地址范围和块头一致性，非法释放直接返回错误。
3. 前后相邻块为空闲时合并，降低外部碎片。
4. 将合并后的空闲块重新插入对应 size class 链表，并更新位图。

**特点**

- 位图 + 分级链表带来较低查找成本，适合默认小型内核场景。
- 节点尾部带 `OS_FSC_MEM_TAIL_MAGIC` 溢出检测，这是 FSC 专有机制；TLSF、bestfit_little、bestfit 不伪造该 magic。
- size class 粒度来自块大小最高有效位，查找快，但相比 best-fit 类算法更容易产生一定内部碎片。

### TLSF 算法

- **TLSF**（Two-Level Segregated Fit，两级分隔适配）：通用动态内存算法，通过一级粗粒度桶和二级细粒度桶管理空闲块，目标是在可控碎片下提供有界查找时间。
- 源码目录 `src/mem/tlsf/`：

| 文件 | 说明 |
|---|---|
| `prt_tlsf_core.c` / `prt_tlsf_core.h` | TLSF 算法核心，提供 `OsTlsfInit`/`OsTlsfAlloc`/`OsTlsfAllocAlign`/`OsTlsfFree`/`OsTlsfRealloc`/`OsTlsfInfoGet` 等 |
| `prt_tlsf_list.h` | 算法自用的双向链表（`OsTlsfList*`/`OS_TLSF_DL_LIST*`） |
| `prt_tlsf_compat.h` | 类型/打印/hook/任务等适配层，直接对接 UniProton 原生 `PRT_` 接口（如 `PRT_HwiLock`/`PRT_Printf`/`PRT_TaskSelf`） |
| `prt_tlsf_config.h` | 算法配置宏（`TLSF_CFG_*`），裁剪 leakcheck/integrity/task-mem/lms/lmk 等可选项 |
| `prt_tlsfmem.c` | 胶水层：`OsTlsfMemInit`（注册到 `g_memArithAPI`）、`OsMemAlloc`/`OsMemAllocAlign`/`OsTlsfMemFree`（包装算法接口 + 维护统计量） |

- **与 FSC 行为对齐**：`size==0` 返回 NULL；对齐 `boundary` 小于指针宽度时钳到 `sizeof(VOID*)`；维护 `g_memUsage`/`g_memPeakUsage`/`g_memTotalSize`/`g_memStartAddr` 统计量供 memInfo 使用。
- **暂不支持 expand**（`OS_MEM_EXPAND_ENABLE=0`），即不动态扩展内存池。

**原理与数据结构**

- 内存池头为 `OsMemPoolHead`，内部包含 `OsMemPoolInfo`、空闲链表数组 `freeList[OS_MEM_FREE_LIST_COUNT]`、一级/二级 bitmap 等信息。
- 每个物理连续节点使用 `OsMemNodeHead` 描述大小、used/aligned/last 等 flag，以及前驱节点指针。
- 空闲节点扩展为 `OsMemFreeNodeHead`，在节点头后附加空闲链表的 `prev`/`next`。
- 小块直接映射到 small bucket；大块先按最高有效位得到 first level，再按 `OS_MEM_SLI` 划分 second level，最终映射到一个空闲链表下标。

**分配流程**

1. `OsTlsfAlloc` 加锁并检查 pool、size 合法性。
2. `OsMemFreeNodeGet` 根据申请大小映射 FL/SL 下标，先查当前桶，当前桶为空时通过 bitmap 找后续非空桶。
3. 找到空闲块后从空闲链表删除；若剩余空间大于最小空闲节点，则 `OsMemSplitNode` 分裂出新空闲节点并重新入链。
4. 设置 used flag、任务 ID 和水位统计，返回用户地址。

**释放流程**

1. `OsTlsfFree` 根据用户地址还原节点头；对齐分配通过 gap size 记录还原真实节点。
2. 校验地址范围、节点 used flag 和完整性。
3. 清除 used flag，并分别尝试和前后相邻空闲节点合并。
4. 合并后的节点通过 `OsMemFreeNodeAdd` 插回对应 FL/SL 空闲链表。

**特点与约束**

- 查找依赖位图和两级桶，复杂度不随空闲块数量线性增长，适合需要稳定分配时延的场景。
- 比 FSC 元数据更复杂，但对不同尺寸申请的碎片控制更细。
- UniProton 当前适配关闭了 expand、LMS、LMK、leakcheck、异常 dump 等依赖外部子系统的可选功能；水位、多池、任务 ID 统计等自包含能力可保留。

### bestfit_little 算法与 slab 扩展

- **bestfit_little**：从 LiteOS bestfit_little 移植到 UniProton 的轻量 best-fit heap 算法，文件按 `prt_*` 命名；算法主体保持原有分配、释放、分裂、合并流程，UniProton 只增加 compat 头、胶水层和必要的接口命名适配。
- 源码目录 `src/mem/bestfit_little/`：

| 文件 | 说明 |
|---|---|
| `prt_bestfit_little_memory.c` / `prt_bestfit_little_heap.c` / `prt_bestfit_little_memory_internal.h` | bestfit_little 算法主体；原 LiteOS `LOS_Mem*` 算法入口在 UniProton 内改为 `PRT_BestfitLittleMem*` |
| `compat/prt_bestfit_little_*.h` | 算法依赖适配到 UniProton：类型、打印、锁、任务统计、配置宏等 |
| `prt_bestfit_littlemem.c` | 胶水层：初始化当前堆、填充 `g_memArithAPI`、维护 UniProton 统计量、提供 `malloc_usable_size` |
| `../slab/prt_slab.c` / `../slab/prt_slabmem.c` | 共享 slab 小块 cache 实现，通过 adapter 绑定 bestfit_little |

**原理与数据结构**

- heap 管理结构为 `LosHeapManager`，记录堆内首尾节点。
- 每个堆节点为 `LosHeapNode`，保存节点大小、used 状态、前驱/后继关系等元数据。
- 分配策略是 best-fit：遍历可用节点，选择能满足申请且剩余空间最小的空闲块，尽量减少一次分配留下的大碎片。

**分配流程**

1. `PRT_BestfitLittleMemAlloc` 先做参数检查和加锁。
2. 开启 slab 时先尝试 `OsSlabMemAlloc`，小块命中则直接返回。
3. slab 未命中或未开启时进入 `OsHeapAlloc`，按 best-fit 规则选择最小可用空闲节点。
4. 如果选中的节点剩余空间足够容纳新节点，则分裂为 used 节点和剩余 free 节点。
5. 设置节点 used 状态和统计信息后返回用户地址。

**释放流程**

1. `PRT_BestfitLittleMemFree` 先尝试 `OsSlabMemFree`，命中 slab 则完成释放。
2. heap 释放通过 `((struct LosHeapNode *)ptr) - 1` 从用户地址还原节点头。
3. 校验节点范围、状态和前后关系，非法释放返回错误。
4. 将当前节点标记为空闲，并与相邻空闲节点合并。

- **slab 架构位置**：slab 不是替代 heap 的完整动态内存算法。开启后，bestfit_little 分配入口先尝试 `OsSlabMemAlloc` 分配小块；命不中再走 bestfit_little 的 `OsHeapAlloc`。释放时先尝试 `OsSlabMemFree`，失败再回落 `OsHeapFree`。
- **接口边界**：UniProton 对外只暴露标准 `PRT_Mem*` 接口和内核内部 `OsMem*` 分发表接口；bestfit_little 算法私有入口使用 `PRT_BestfitLittleMemInit`/`PRT_BestfitLittleMemAlloc`/`PRT_BestfitLittleMemFree` 等 `PRT_` 前缀，不继续暴露 LiteOS 的 `LOS_Mem*` 接口名。
- **符号边界**：slab 内部使用 `OsMemAlloc`/`OsMemFree` 作为 backing heap hook；UniProton 已有同名公共接口，因此 compat 层把 slab 内部 hook 重命名为 `OsPrtHeapAllocForSlab`/`OsPrtHeapFreeForSlab`，避免改变 UniProton 公共架构。
- **64 位适配**：bestfit_little 的 `OsHeapFree` 通过 `((struct LosHeapNode *)ptr) - 1` 还原节点头。64 位下需要 `LOSCFG_MEM_TASK_STAT` 字段使 `data[0]` 偏移与 `sizeof(struct LosHeapNode)` 保持一致；compat 层开启该字段并通过 UniProton 任务接口提供任务 ID。

### bestfit 算法

- **bestfit**：从 LiteOS bestfit 移植到 UniProton 的完整 bestfit 分配算法，作为独立算法分支由 `CONFIG_OS_MEM_ARITH_BESTFIT` 选择。移植时保持算法主体的分配、释放、分裂、合并和完整性检查流程，UniProton 侧只做命名、依赖适配和公共分派接入。
- 源码目录 `src/mem/bestfit/`：

| 文件 | 说明 |
|---|---|
| `prt_bestfit_memory.c` / `prt_bestfit_memory_internal.h` | bestfit 算法主体；算法私有入口在 UniProton 内统一为 `PRT_BestfitMem*` |
| `prt_bestfit_multipledlinkhead.c` | 多级空闲链表头选择与初始化 |
| `compat/prt_bestfit_*.h` | 类型、链表、打印、锁、任务、LMS stub、配置宏等 UniProton 适配层 |
| `prt_bestfitmem.c` | 胶水层：初始化当前堆、填充 `g_memArithAPI`、维护 UniProton 统计量、提供 `malloc_usable_size` |
| `../slab/prt_slab.c` / `../slab/prt_slabmem.c` | 共享 slab 小块 cache 实现，通过 adapter 绑定 bestfit |

**原理与数据结构**

- 内存池头为 `PrtBestfitMemPoolInfo`，包含 pool 基本信息、统计信息以及 slab 控制头（开启 slab 时使用）。
- 堆节点为 `PrtBestfitMemDynNode`，包含 `selfNode` 和 `backupNode` 两份控制信息；控制节点 `PrtBestfitMemCtlNode` 记录 size/flag、前驱节点、空闲链表节点、任务 ID、模块 ID 等。
- 空闲块不是单链遍历，而是挂在多级双向链表头上；`PrtBestfitMultipleDlinkHead` 和 `prt_bestfit_multipledlinkhead.c` 负责按大小选择链表头。
- used/free 状态、对齐 gap、magic、checksum、任务信息都保存在节点控制区中，用于释放、完整性检查和统计。

**分配流程**

1. `PRT_BestfitMemAlloc` 检查参数并加锁。
2. 开启 slab 时先进入 `OsSlabMemAlloc`，小块命中则跳过 heap。
3. heap 分配根据申请大小选择合适的多级空闲链表，查找最匹配的可用节点。
4. 选中节点后从空闲链表摘除；剩余空间足够时分裂，并把剩余节点重新插入对应链表。
5. 设置 used flag、magic/checksum、任务 ID 和统计信息，返回用户地址。

**释放流程**

1. `PRT_BestfitMemFree` 先检查 slab，slab 命中则直接释放。
2. heap 释放根据用户地址和 gap 信息还原 `PrtBestfitMemDynNode`。
3. 校验节点 magic、checksum、used 状态和地址范围。
4. 标记为空闲，和前后相邻空闲节点合并，然后插回多级空闲链表。

- **接口边界**：对外仍只暴露标准 `PRT_Mem*` 接口和内核内部共享 `OsMem*` 分发表接口；bestfit 算法私有入口使用 `PRT_BestfitMemInit`/`PRT_BestfitMemAlloc`/`PRT_BestfitMemAllocAlign`/`PRT_BestfitMemFree`/`PRT_BestfitMemRealloc` 等 `PRT_` 前缀。
- **OS 依赖适配**：`PrtBestfitCurrTaskGet` 通过 `PRT_TaskSelf` 和 `PRT_TaskGetInfo` 获取当前任务 TCB，不再使用空实现；锁、打印、panic、链表和 memcheck 常量由 bestfit 私有 compat 层提供。
- **slab 关系**：bestfit 与 bestfit_little 一样支持 `CONFIG_OS_MEM_SLAB_EXTENTION`。slab 仍作为 heap 扩展挂在 bestfit 分配入口前面：小块先尝试 `OsSlabMemAlloc`，命不中再走 bestfit heap；释放时先尝试 `OsSlabMemFree`，失败再回落 bestfit heap free。
- **自动扩展模式**：bestfit slab 同样支持 `CONFIG_OS_MEM_SLAB_AUTO_EXPANSION_MODE`。开启后 slab bucket 从 bestfit heap 申请和释放，算法公共分派与 bestfit heap 主流程不变。
- **64 位适配**：slab 私有块头在 UniProton 适配中补齐到 8 字节，保证 AArch64 下 `PRT_MemAlloc` 返回的小块地址满足指针宽度对齐要求；算法的分配/释放/位图管理流程保持不变。

### slab 小块扩展

slab 是 bestfit_little 和 bestfit 的小块 cache 扩展能力，不作为独立 `OS_MEM_ARITH_*` 算法。它的目标是把高频小尺寸申请从通用 heap 路径前置出来，减少小块反复分裂/合并带来的碎片和时延。

**原理与数据结构**

- slab class 数量为 `SLAB_MEM_COUNT=4`，步长为 `SLAB_MEM_CALSS_STEP_SIZE=0x10`，默认覆盖 16、32、64、128 字节小块。
- `OsSlabBlockNode` 位于用户地址前，用于记录 slab magic、块大小和 record id；释放和 `malloc_usable_size` 依赖该头部识别 slab 块。
- `OsSlabAllocator` 管理同一 item size 的连续 item 区域，`AtomicBitset` 记录 item 是否已使用。
- `OsSlabMem` 表示一个 slab class，记录块大小、块数量、使用计数。自动扩展模式下还包含 `OsSlabMemAllocator` bucket 链。
- slab 实现只有一份，位于 `src/mem/slab/`。`prt_slab_adapter.h` 根据 `OS_MEM_ARITH_BESTFIT_LITTLE` 或 `OS_MEM_ARITH_BESTFIT` 选择对应 compat 头，把控制头类型、状态类型、返回码、自动扩展宏和 backing heap alloc/free hook 适配到当前 heap。
- bestfit_little 和 bestfit 各有自己的 compat 头，分别把控制头嵌入对应 heap pool，不改变 UniProton 公共内存架构。

**分配流程**

1. backing heap 算法入口收到申请后，若 `size <= SLAB_MEM_MAX_SIZE`，先进入 `OsSlabMemAlloc`。
2. 按申请大小选择最小可容纳 class。
3. 在 class 对应 allocator 的 bitset 中查找 clear bit 并置位。
4. `OsSlabBlockHeadFill` 写入 magic 和实际 class 大小，返回 block header 之后的用户地址。
5. slab class 已满时，固定模式返回 `NULL` 并回落 heap；自动扩展模式尝试申请新 bucket。

**释放与检查流程**

1. 释放时先通过 `OS_SLAB_BLOCK_HEAD_GET(ptr)` 取 slab header。
2. 若 magic 不匹配，说明该指针不是 slab 分配，返回失败并交给 backing heap 释放。
3. magic 匹配后，根据块大小找到 class，并通过 `OsSlabAllocatorFree` 清 bit。
4. `OsSlabMemCheck` 用相同逻辑判断指针是否属于 slab，并返回 class 块大小供 `malloc_usable_size` 使用。

**自动扩展模式**

- 宏控：`CONFIG_OS_MEM_SLAB_AUTO_EXPANSION_MODE=y`，依赖 `CONFIG_OS_MEM_SLAB_EXTENTION=y`。
- bestfit_little 适配为 `LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE`；bestfit 适配为 `PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE`。
- 未开启时，每个 class 只有固定 allocator，用尽后小块申请回落 backing heap。
- 开启后，每个 class 维护 bucket 链；当前 bucket 满时从 backing heap 申请新的 `OsSlabMemAllocator` 和对应 `OsSlabAllocator`。
- 释放后若某个扩展 bucket 已空，且 class 的 bucket 数量大于默认 bucket 数，释放该 bucket，避免 slab 只增不减。

### TLSF 配置宏

TLSF 算法的可选项由 `prt_tlsf_config.h` 的 `TLSF_CFG_*` 宏控制（`prt_tlsf_core.c` 中以 `#if` 守护）。适配时已为所有宏设置安全默认值。下面按「能否开启」分类说明。

#### 当前无法开启（依赖的子系统未适配到 UniProton）

开启下列任一宏会引入当前不存在的依赖，**必须保持关闭**：

- **`OS_MEM_EXPAND_ENABLE`**（在 `prt_tlsf_core.c` 中硬编码为 0）
  - 功能：内存池耗尽时动态向系统申请新页扩展池容量。
  - 缺失依赖：页分配器 `OsTlsfPhysPagesAlloc`、`PAGE_SHIFT`、`OsTryShrinkMemory`。

- **`TLSF_CFG_KERNEL_LMS`**（默认不定义，`#ifdef` 判断）
  - 功能：LMS（Lite Memory Sanitizer）内存越界 / use-after-free 检测。
  - 缺失依赖：LMS 模块。

- **`TLSF_CFG_KERNEL_LMK`**（默认 0）
  - 功能：LMK，内存耗尽时按策略 kill 任务以回收内存。
  - 缺失依赖：`OsTlsfLmkTasksKill`。

- **`TLSF_CFG_MEM_LEAKCHECK`**（默认 0）
  - 功能：内存泄漏检查，为每个分配节点记录调用栈（返回地址）。
  - 缺失依赖：回溯接口 `OsBackTraceHookCall`；配套宏 `TLSF_CFG_MEM_RECORD_LR_CNT` / `TLSF_CFG_MEM_OMIT_LR_CNT` 同样不可开。

- **`TLSF_CFG_PLATFORM_EXC`**（默认 0）
  - 功能：与异常子系统联动，异常时 dump 内存池信息。
  - 缺失依赖：异常 dump 适配模块。

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
  - 注意：可正常编译；完整性失败 panic 路径中的任务信息查询（`OS_TLSF_TCB_FROM_TID`）已通过 `PRT_TaskGetInfo` 对接 UniProton 任务信息。

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
    prt_bestfit_littlemem_external.h bestfit_little 算法声明
    prt_bestfitmem_external.h bestfit 算法声明
  fsc/                   FSC 算法实现
  tlsf/                  TLSF 算法实现
  bestfit_little/        bestfit_little 算法与 slab 扩展
  bestfit/               bestfit 算法实现
  slab/                  bestfit_little/bestfit 共享 slab 扩展实现与 adapter
```

### 新增算法

新增一种内存分配算法（如 bestfit）的步骤：

1. 在 `src/mem/<algo>/` 下实现自己的初始化 `Os<Algo>MemInit` 与释放 `Os<Algo>MemFree`，以及共享名的 `OsMemAlloc`/`OsMemAllocAlign`，并在 init 中填充 `g_memArithAPI` 分发表。
2. 在 `src/mem/Kconfig` 增加 `config OS_MEM_ARITH_<ALGO>`。
3. 在 `src/mem/CMakeLists.txt` 增加对应编译分支。
4. 在 `prt_mem.c` 的 `OsMemInit` 中增加 `#elif defined(OS_MEM_ARITH_<ALGO>)` 分派分支。
5. 板级 defconfig 设置该宏即可切换，无需改动 `OsMemConfigReg`。

如果新增的是 slab 这类 heap 扩展能力，不应作为 `OS_MEM_ARITH_<ALGO>` 独立分支接入；应挂在对应 backing heap 算法内部，并用独立扩展宏控制。

## 开发流程
### 步骤一：设置内存管理模块配置项

使用UniProton内存管理模块，需要进行配置项的设置，需要配置的项包括缺省分区首地址、分区大小等。

### 步骤二：使用内存管理模块

当需要使用内存时，需要先创建一个指定内存管理算法的内存分区，通过调用内存申请接口申请合适大小的内存，就可以对申请到的内存进行操作（包括写操作，然后给其他模块传递消息等）；如果是动态内存，当内存使用完，需要对这块内存进行释放，防止发生内存泄漏。

## 测试

### 测试分类原则

由于各算法仅底层实现不同、对外标准接口一致，因此**绝大部分测试用例是算法无关的（common）**，FSC、TLSF、bestfit_little、bestfit 等已接入算法都应通过；只有极少数用例依赖某算法专有机制，归为该算法独有：

| 分类 | 说明 | 运行条件 |
|---|---|---|
| **common** | 经标准接口（`PRT_MemAlloc`/`PRT_MemFree`/`PRT_MemAllocAlign` 或 libc malloc）验证分配/释放/对齐/复用等功能，不依赖任何算法私有布局 | 所有已接入算法都跑 |
| **FSC 专有** | 依赖 FSC 私有机制（如节点尾部 `OS_FSC_MEM_TAIL_MAGIC` 溢出检测） | 仅 FSC 跑 |
| **TLSF 专有** | 依赖 TLSF 私有机制 | 仅 TLSF 跑 |

### 测试 app

| app | 源文件 | 覆盖 | 说明 |
|---|---|---|---|
| `UniPorton_test_posix_malloc_interface` | `runMallocTest.c` + posix malloc 用例 | libc malloc/calloc/realloc/memalign/reallocarray/usable_size 一致性 | common，经 libc→`PRT_Mem` |
| `UniPorton_test_prt_mem_interface` | `runPrtMemTest.c` | `PRT_Mem*` 直接接口 | common，直接验证 `PRT_MemAlloc`/`Free`/`AllocAlign` |

`runPrtMemTest` 当前 16 个 common 用例：

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
| prt_mem_011 | 重复释放必须返回错误 |
| prt_mem_012 | 非法对齐枚举返回 NULL |
| prt_mem_013 | `malloc_usable_size` 普通分配结果覆盖申请大小 |
| prt_mem_014 | 对齐分配可被 `malloc_usable_size` 正确识别 |
| prt_mem_015 | 小块批量分配释放，覆盖 slab/小块碎片路径，并校验返回地址满足指针宽度对齐 |
| prt_mem_016 | 新建任务上下文中分配/释放，验证任务信息适配 |

### 算法独有用例

- **`malloc_usable_size_1_1`**（FSC 专有）：校验 `*(U32*)(ptr + usableSize) == OS_FSC_MEM_TAIL_MAGIC`，依赖 FSC 的 tail magic 机制。在 `runMallocTest.h` 中仅 FSC 模式登记该用例，TLSF/bestfit_little/bestfit 模式下排除（这些算法无 FSC tail magic 机制，为保证算法原有行为不伪造 magic）。

### 验证结果（sd3403，armv8）

多种算法均上板验证，结果如下：

| 算法（defconfig） | prt_mem（common 16） | malloc（一致性） |
|---|---|---|
| bestfit_little + slab（`CONFIG_OS_MEM_ARITH_BESTFIT_LITTLE=y` + `CONFIG_OS_MEM_SLAB_EXTENTION=y`） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| bestfit_little + slab auto expansion（再启用 `CONFIG_OS_MEM_SLAB_AUTO_EXPANSION_MODE=y`） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| bestfit + slab（`CONFIG_OS_MEM_ARITH_BESTFIT=y` + `CONFIG_OS_MEM_SLAB_EXTENTION=y`） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| bestfit + slab auto expansion（再启用 `CONFIG_OS_MEM_SLAB_AUTO_EXPANSION_MODE=y`） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| bestfit（`CONFIG_OS_MEM_ARITH_BESTFIT=y`，不启用 slab） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| TLSF（`CONFIG_OS_MEM_ARITH_TLSF=y`） | 16/16 通过 | 10/10 通过（FSC 专有 usable_size 用例排除） |
| FSC（缺省） | 10/10 通过 | 11/11 通过（含 usable_size） |

结论：common 用例在已接入算法分支均通过；FSC 专有的 `malloc_usable_size_1_1` 仅在 FSC 模式运行并通过；`OsMemInit` 公共分派的 FSC/TLSF/bestfit_little/bestfit 分支均已覆盖构建或上板验证。

### 上板运行（sd3403）

构建（容器内）：
```bash
cd demos/sd3403/build
sh build_app.sh UniPorton_test_prt_mem_interface
sh build_app.sh UniPorton_test_posix_malloc_interface
```
切换算法：修改 `build/uniproton_config/config_armv8_sd3403/defconfig` 中 `CONFIG_OS_MEM_ARITH_TLSF` 后重新构建。部署/运行/采集串口日志/清理的生命周期管理按 sd3403 deploy 流程（SCP 传 ELF → mica 配置 CPU3 → 触发 → 采集 → `mica stop`/`rm`）。

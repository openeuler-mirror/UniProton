# UniProton Perf使用指南

## 1 Perf功能概述
Perf是一种协助分析性能瓶颈的工具，在打开监测功能后可随业务运行过程实现监测。诸如cache命中率、任务切换栈、内存分配情况等信息，并以非易失性介质存储以方便后续分析优化。

## 2 Perf总体方案
支持三种监测方式：
1) 监测硬件层面如cache命中等，由于Perf是硬件相关，需要考虑做好硬件抽象层，在此方式下通过MSR/MRS指令读写PMU相关寄存器来清除、设置相关计数器，在中断触发时收集当前任务信息，用于最终汇总分析。
2) 监测软件层面如任务切换，此方式下在任务切换、进入及退出中断时收集任务信息。
3) 通过软件定时器监测CPU，此方式下定时器溢出时收集任务相关信息。
性能监测模块不影响原有功能，可通过宏开关裁剪，三种监测方式统一调度框架。但由于Perf在任务正常运行之外需要收集相关信息用于分析，不可避免的对实时侧业务性能造成部分影响，因此建议用户在感知到性能较差需要分析原因时才主动打开Perf模块，并进行分析优化，优化后关闭此功能。

## 3 编译时使能Perf功能
defconfig 设置
```
CONFIG_OS_OPTION_PERF=y            # 使能Perf总开关，使用Perf功能必须开启

# 以下三个开关在使用对应监测方式时必须开启，且同时只能开启一个
CONFIG_OS_OPTION_PERF_HW_PMU=y     # 使能硬件监测
CONFIG_OS_OPTION_PERF_SW_PMU=y     # 使能软件监测
CONFIG_OS_OPTION_PERF_TIMED_PMU=y  # 使能定时器监测
```
如果选用定时器监测方式，还需在prt_config.h中将OS_INCLUDE_TICK_SWTMER打开。

## 4 Perf功能接口使用
Perf的主要功能可参照prt_perf_demo.c中的测试demo使用。

### 4.1 PRT_PerfInit
在使用Perf功能时，需首先调用PRT_PerfInit进行初始化操作。在该接口中对环形缓冲区进行内存申请和初始化，申请大小由用户指定；对硬件监测方式做中断回调注册；对start、stop、config等不同操作按监测方式做钩子注册。

### 4.2 PRT_PerfConfig
初始化完成后，调用PRT_PerfConfig传入用户的配置项，配置项在PerfConfigAttr结构体中设置。可设置监测类型、监测事件、周期、采样类型等，并关联相关PMC。

### 4.3 PRT_PerfStart
配置完成，调用PRT_PerfStart清空并使能PMC相关寄存器信息，开启监测。按周期采集pc、函数调用栈等信息，采集信息写入环形缓冲区，达到一定量后使用提供的钩子通知用户。

### 4.3 PRT_PerfStop
停止监测，对寄存器去使能等。

## 5 监测信息输出
在监测过程中，采样信息是写入环形缓冲区的。如果要输出采样信息，需要调用PRT_PerfDataRead将环形缓冲区的数据读取到指定缓冲区，并通过OsPrintBuff将缓冲区数据打印输出。对于计数信息，在PRT_PerfStop中可直接打印。

## 6 火焰图生成
基于采样信息中的调用栈信息，可生成对应的火焰图，从而分析函数执行时间并进行优化。默认情况下，调用栈采样信息是输出到测试单板的/tmp/output.perf文件中，将该文件拷贝到本地linux系统，并使用FlameGraph工具生成火焰图：
```
// 工具下载
git clone https://github.com/brendangregg/FlameGraph.git

// 火焰图生成
./FlameGraph/stackcollapse-perf.pl output.perf > output.folded
./FlameGraph/flamegraph.pl output.folded > output.svg
```
当前生成的火焰图还不支持符号显示，需将显示的地址根据build目录下的反汇编文件手动反推符号信息。


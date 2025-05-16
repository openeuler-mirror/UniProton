# Uniproton testsuites使用指南

## 1 Uniproton testsuites用例：
UniProton跟测试相关代码放在UniProton/testsuites目录下，如果想运行对应的测试用例来测试性能和指标，需要编译对应测试套的二进制文件。

## 2 Uniproton testsuites组成架构：
以ascend310b posixtestsuite测试malloc为例，概述测试用例如何运行：

(1) 修改demos/ascend310b/CMakeLists.txt文件，将UniProton/testsuites/posixtestsuite/conformance中将相关的代码加入编译：
```
if (${APP} STREQUAL "UniPorton_test_posix_malloc_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
```
(2) 修改UniProton/demos/ascend310b/apps/openamp/main.c文件，在创建的任务中添加测试套任务入口：
```
#if defined(POSIX_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(POSIX_TESTCASE)
    PRT_Printf("init testcase\n");
    Init(0, 0, 0, 0);
#endif
```
(3) testsuites/posixtestsuite/conformance/CMakeLists.txt，会根据编译的bin文件名称，将对应测试套源码编译：
```
if (${APP} STREQUAL "UniPorton_test_posix_malloc_interface")
    set(BUILD_APP "UniPorton_test_posix_malloc_interface")
    set(ALL_SRC runMallocTest.c ${ALL_MALLOC_SRC})
```

(4)malloc测试用例会将testsuites/posixtestsuite/conformance/runMallocTest.c文件编译到bin文件中,Init入口依次运行run_test_arry_1中的测试用例：
```
test_run_main *run_test_arry_1[] = {
	malloc_calloc_1_1,
	malloc_malloc_1_1,
	malloc_malloc_2_1,
	malloc_malloc_3_1,
	malloc_memalign_1_1,
	malloc_memalign_2_1,
	malloc_realloc_1_1,
	malloc_realloc_2_1,
	malloc_realloc_3_1,
	malloc_reallocarray_1_1,
	malloc_usable_size_1_1
};
```

(5) 最后需要修改UniProton/demos/ascend310b/build/build_app.sh中APP，然后sh +x build_app.sh编译即可：
```
export APP=UniPorton_test_posix_malloc_interface
```

## 3 Uniproton testsuites新增测试用例：
整个编译框架已搭建好，如果用户想新增malloc测试用例，在run_test_arry_1数组中新增测试套实现即可。如果想新增其他类型的测试套，需要根据编译选项找到测试套具体执行的Init入口函数，在对应的数组中新增测试套实现加入编译即可。

## 4 Uniproton testsuites用例支持情况：
* posixtestsuite
* rhealstone
* drivers
* shell
* libc
* libxml2
* log
* forte
* cxx
* eigen
* modbus
* soem
* lwip
* eigen
* opcua
* jitter
* cyclictest

## 5 UniProton 性能指标测试方法
UniProton性能测试方法包括rhealstone、cyclictest、jitter-test三种：

### 5.1 rhealstone
rhealstone用于评估实时系统在处理一系列典型实时任务时的性能表现，包括死锁解除、中断响应、消息传输、信号量混洗、任务抢占、上下文切换六方面。

#### 5.1.1 deadlock-break
测试死锁解除时延。即系统从检测到死锁发生到完全解除死锁并恢复任务正常执行的耗时。

#### 5.1.2 interrupt-latency
测试中断响应时延。通过计算从触发核间SGI中断到进入ISR之间的CPU周期数，获取系统的中断响应时间。

#### 5.1.3 message-latency
测试消息传输时延。两个任务之间通过消息队列传输消息，测量消息从发送方写入队列到接收方获取到消息之间的CPU周期数，多次测试取平均值。

#### 5.1.4 semaphore-shuffle
测试信号量混洗时延。创建优先级相同的两个任务，两个任务竞争同一信号量。其中一个任务释放信号量后，测量另一个任务从等待状态到被唤醒并成功获取信号量的时间差。其测试步骤如下：
1) 任务A获取信号量后进入阻塞等待。
2) 任务B释放信号量，触发任务A唤醒。
3) 记录从释放到获取完成的CPU周期数，多次测试取平均值。

#### 5.1.5 task-preempt
测试任务抢占时延。通过创建优先级不同的两个任务，计算将CPU控制权从低优先级任务转移到高优先级任务所需的时间。测试步骤如下：
1) 创建优先级不同的两个任务，分别为低优先级任务A和高优先级任务B。
2) 将任务B挂起，并在任务A重恢复任务B，使其返回就绪队列。
3) 记录从恢复任务到任务B实际开始执行的CPU周期数，多次测试取平均值。

#### 5.1.6 task-switch
测试上下文切换时延。通过创建优先级相同的两个任务，每个任务都会主动挂起自身并唤醒对方，形成循环切换，计算上下文切换所需的平均时间。

#### 5.1.7 用例测试方式
以deadlock-break用例为例，编译命令为
```
sh build_app.sh deadlock-break
```
编译完成后，将deadlock-break.elf上传至测试单板，使用mica拉起后，通过screen命令进入UniProton shell界面，回车后便可显示测试结果。其他用例测试可参照上述方式。

### 5.2 cyclictest
cyclictest是针对实时系统的延迟测试工具。通过在实时任务中设置定时器进入睡眠，并在唤醒后计算实际唤醒时间与预期时间的差值。该时间差包含tick中断的响应时间和任务唤醒后被重新调度恢复上下文的时间，即系统中断延时与调度延时之和。在测试中对以上步骤进行多次循环，分别取最大、最小以及平均延时并打印输出。其测试步骤为：
1) 设置测试周期与循环次数，测试周期即为每次循环中实时任务的定时睡眠时间，当前默认周期为1000us，默认循环次数为1000。以hi3093平台为例，可在UniProton/demos/hi3095/apps/openamp/main.c中修改配置：
```
#if defined(CYCLIC_TESTCASE)
    cyclictest_entry(1000, 1000); // 第一个参数为测试周期，第二个参数为循环次数
#endif
```
2) 编译cyclictest测试demo，编译命令为：
```
sh build_app.sh UniProton_cyclictest
```
3) 编译完成后，将UniProton_cyclictest.elf上传至测试单板，使用mica拉起后进入shell界面，回车运行便可显示测试结果。

### 5.3 jitter-test
该用例测试的场景为实时内核在125us控制周期下的中断时延抖动，测试方式是通过构造周期为125us的cpu系统定时器，当系统定时器溢出时，触发定时器中断，并在中断ISR的入口处统计CPU周期数。假定系统定时器运行的CPU周期数为设定值c1，前后两次进入ISR的CPU周期间隔为c2，c2与c1的差值即为系统的外部中断时延，通过多次循环测试，记录时延的最大、最小和平均值，可得到测试的抖动情况。其测试步骤为：
1) 编译测试demo：
```
sh build_app.sh UniProton_jitter_test
```
2) 编译完成后，将UniProton_jitter_test.elf上传至测试单板，使用mica拉起后便可进入shell界面查看测试结果。

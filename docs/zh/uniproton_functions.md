# UniProton功能设计

<!-- TOC -->

- [UniProton功能设计](#uniproton功能设计)

    - [支持任务管理](#支持任务管理)

    - [支持事件管理](#支持事件管理)

    - [支持队列管理](#支持队列管理)

    - [支持硬中断管理](#支持硬中断管理)

    - [支持内存管理](#支持内存管理)

        [FSC内存算法](#fsc内存算法)

        [核心思想](#核心思想)

        [内存申请](#内存申请)

        [内存释放](#内存释放)

    - [支持定时器管理](#支持定时器管理)

    - [支持信号量管理](#支持信号量管理)

    - [支持异常管理](#支持异常管理)

    - [支持CPU占用率统计](#支持cpu占用率统计)

    - [支持STM32F407ZGT6开发板](#支持stm32f407zgt6开发板)

    - [支持OpenAMP混合部署](#支持openamp混合部署)

    - [支持POSIX标准接口](#支持posix标准接口)

    <!-- TOC -->

## 支持任务管理

UniProton是一个单进程支持多线程的操作系统。在UniProton中，一个任务表示一个线程。UniProton中的任务为抢占式调度机制，而非时间片轮转调度方式。高优先级的任务可打断低优先级任务，低优先级任务必须在高优先级任务挂起或阻塞后才能得到调度。

UniProton的任务一共有32个优先级(0-31)，最高优先级为0，最低优先级为31。每个优先级可以创建多个任务。

UniProton任务管理模块提供任务创建、任务删除、任务挂起、任务恢复、任务延时、锁任务调度、解锁任务调度、当前任务ID获取、任务私有数据获取与设置、查询指定任务正在Pending的信号量ID、查询指定任务状态、上下文信息、任务通用信息、任务优先级设定与获取、调整指定优先级的任务调度顺序、注册及取消任务创建钩子、任务删除钩子、任务切换钩子等功能。UniProton在初始化阶段，默认会创建一个最低优先级的IDLE任务，用户在没有处于运行态的任务时，IDLE任务被运行。

## 支持事件管理

事件机制可以实现线程之间的通讯。事件通讯只能是事件类型的通讯，无数据传输。 

UniProton事件作为任务的扩展，实现任务之间的通讯。每个任务支持32种类型事件（32个 bit位，每bit代表一种事件类型）。 

UniProton提供读取本任务事件和写指定任务事件的功能。读事件时可以同时读取多种事件，也可以只读取一种事件，写事件时也可以同时写一种或多种类型事件。

## 支持队列管理

队列（Queue），又称消息队列，是线程间实现通信的一种方式，实现了数据的存储和传递功能。根据优先级可以将数据写入到队列头或队列尾，但只能从队列的头处读取数据。

UniProton创建队列时，根据用户传入队列长度和消息单元大小来开辟相应的内存空间以供该队列使用。在队列控制块中维护一个头指针Head和一个尾指针Tail来表示当前队列中数据存储情况。头指针Head表示队列中被占用消息的起始地址，尾指针Tail表示队列中空闲消息的起始地址。

## 支持硬中断管理

硬中断是由硬件触发的会改变系统运行轨迹的一个电平信号，硬中断用于通知CPU某个硬件事件的发生。硬中断一般分为可屏蔽中断和不可屏蔽中断（NMI）两种。

硬中断的优先级高于所有任务，其内部也有不同的优先级，当同时有多个硬中断被触发时，最高优先级的硬中断总是优先得到响应。高优先级硬中断是否能打断正在执行的低优先级硬中断（即中断嵌套），视不同芯片平台而异。

出于任务延时、软件定时器等需要，OS会在初始化阶段，创建1个Tick硬中断，其实质是一个周期性的硬件定时器。

## 支持内存管理

内存管理主要工作是动态的划分并管理用户分配好的大片内存区间。当程序某一部分需要使用内存，可以通过操作系统的内存申请函数索取指定大小内存块，一旦使用完毕，通过内存释放函数归还所占用内存，使之可以重复使用。

目前UniProton提供了FSC内存算法，该算法优缺点及应用场景如下表所示：

| 内存算法                                                     | 优点                                                         | 缺点                           | 应用场景                             |
| :----------------------------------------------------------- | ------------------------------------------------------------ | ------------------------------ | ------------------------------------ |
| <span style="display:inline-block;width:70px">类型</span>私有FSC算法 | 内存控制块信息占用内存较少，支持最小4字节对齐的内存块大小申请；支持相邻内存块的快速分割合并，无内存碎片。 | 内存申请和内存释放的效率较低。 | 能够灵活适应各种产品的场景。 |

如下简要描述一下FSC内存算法：

### FSC内存算法

#### 核心思想

对于申请的内存大小为uwSize，如果用二进制，则表示为0b{0}1xxx，{0}表示1前面可能有0个或多个零。无论1后面xxx为何内容，如果将1变成10，xxx则全部变成0，则总会出现10yyy > 1xxx（此处yyy表示xxx的对应位全部变成0）。

我们可以直接得到最左边的1的下标。下标值或者从高位到低位依次为0-31（BitMap），或者从低位到高位依次为0-31（uwSize）。如果32位寄存器从高位到低位的bit位的下标依次为0-31，则0x80004000的最左边1的下标为0。于是我们可以维护一个空闲链表头数组（元素数不超过31），以内存块大小最左边的1的下标做为链表头的数组索引，即将所有最左边的1的下标相同的内存块挂接在同一个空闲链表中。

如：索引为2的链表可挂接的空闲块大小为4、5、6、7；索引为N的链表可挂接的空闲块大小为2^N到2^(N+1)-1。

![](./figures/FCS.png)                            

#### 内存申请

当申请uwSize大小的内存时，首先利用汇编指令得到最左边的1的下标，假定为n。为确保空闲链表中的第一个空闲内存块满足uwSize，从索引为n+1开始搜索。若n+1所属空闲链表不为空，则取该链表中的第一个空闲块。若n+1链表为空，则判断n+2链表，依次类推，直到找到非空链表或索引达到31。

为避免for循环逐级判断空闲链表是否为空，定义一个32位的BitMap全局变量。若索引n的空闲链表非空，则BitMap的下标为n的位置1，否则清0。BitMap的下标为31的位在初始化时直接置1。于是查找从n+1开始的第一个非空闲链表，可以首先将BitMap复本的0到n位清零，然后获取复本的最左边的1的下标，若不等于31，即为第一个空闲链表非空的数组索引。

所有的空闲块都以双向链表的形式，串接在空闲链表中。若从链表中获取的第一个空闲块比较大，即分割出一个uwSize的内存块后，剩下的空间至少可做一次最小分配，则将剩余的空闲块调整到对应的空闲链表中。

 ![](./figures/MemoryApplication.png)     

内存控制头中记录空闲内存块的大小（包括控制头本身）。内存控制头中有一个复用成员，位于最首部。当内存块空闲时，作为指向后一个空闲内存块的指针；当内存块占用时，存放魔术字，表示该内存块非空闲。为避免魔术字与指针冲突（与地址值相同），高低4位均为0xf。因为已分配的内存块起始地址需按4字节对齐，所以不存在冲突。

#### 内存释放

当释放内存时，需要将前后相邻的空闲块进行合并。首先，通过判断控制头中的魔术字，确认地址参数（pAddr）的合法性。通过首地址加偏移值的方式，得到后邻的内存块控制头的起始地址。若后邻内存块是空闲的，则将后邻内存块从所属空闲链表中删除，调整当前内存块的大小。

为了使内存释放时能迅速找到前邻的内存块控制头，及判断前邻的内存块是否空闲。内存控制头中增加一个成员，标记前邻的内存块是否空闲。可在内存申请时，将后邻的该标记设置为占用态（若空闲内存块被分割成两块，前一块为空闲，将当前内存块的该标记设置为空闲态）；在内存释放时，将后邻的该标记设置为空闲态。释放当前内存时，若前邻的内存块标记为使用，则不需要合并前邻的内存块；若前邻的内存块标记为空闲，则需要进行合并。若某个内存块为空闲时，则将其后邻控制块的标记设为到本控制块的距离值。

   ![](./figures/MemoryRelease.png)

## 支持定时器管理

定时器管理是为满足产品定时业务需要，UniProton提供了软件定时器功能。

对于软件定时器，是基于Tick实现，所以定时周期必须为Tick的整数倍，在Tick处理函数中进行软件定时器的超时扫描。

目前提供的软件定时器接口，可以完成定时器创建，启动，停止，重启，删除操作。

## 支持信号量管理

信号量（Semaphore）常用于协助一组互相竞争的任务来访问临界资源，在需要互斥的场合作为临界资源计数使用，根据临界资源使用场景分为核内信号量和核间信号量。

信号量对象有一个内部计数器，它支持如下两种操作： 

- 申请（Pend）：Pend 操作等待指定的信号量，若其计数器值大于0，则直接减1返回成功。否则任务阻塞，等待其他线程发布该信号量，等待的容忍时间可设定。 

- 释放（Post）：Post操作发布指定的信号量，若无任务等待该信号量，则直接将计数器加1返回。否则唤醒为此信号量挂起的任务列表中的第一个任务（最早阻塞的）。 

通常一个信号量的计数值用于对应有效的资源数，表示剩余可被占用的互斥资源数。其值的含义如下有两种情况： 

- 为0值：表示没有积累下来的Post操作，且有可能有在此信号量上阻塞的任务。 

- 为正值：表示有一个或多个Post下来的发布操作。 

## 支持异常管理

UniProton中的异常接管属于维测特性，其主要目的是在系统出现异常后，记录尽可能多的异常现场信息，便于后续问题定位。同时提供异常时的钩子函数，便于用户能够在异常发生时做一些用户化的特殊处理。其主要功能是接管内部异常处理或者外部硬件异常。

## 支持CPU占用率统计

UniProton中的系统CPU占用率（CPU Percent）是指周期时间内系统的CPU占用率，用于表示系统一段时间内的闲忙程度，也表示CPU的负载情况。系统CPU占用率的有效表示范围为0～10000，其精度为万分比。10000表示系统满负荷运转。

UniProton中的线程CPU占用率指单个线程的CPU占用率，用于表示单个线程在一段时间内的闲忙程度。线程CPU占用率的有效表示范围为0～10000，其精度为万分比。10000表示在一段时间内系统一直在运行该线程。单核系统所有线程（包括中断和空闲任务）的CPU之和为10000。

UniProton的系统级CPU占用率依赖于Tick模块，通过Tick采样IDLE任务或IDLE软中断计数来实现

## 支持STM32F407ZGT6开发板

支持开发板主要涉及OS内核外围的启动流程和单板驱动，目录结构如下：

├─apps         # 基于UniProton实时OS编程的demo程序。

│ └─hello_world   # hello_world示例程序。

├─bsp          # 提供的板级驱动与OS对接。

├─build          # 提供编译脚本编译出最终镜像。

├─config         # 配置选项，供用户调整运行时参数。

├─include         # UniProton实时部分提供的编程接口API。

└─libs          # UniProton实时部分的静态库，build目录中的makefile示例已经将头文件和静态库的引用准备好，应用可直接使用。

## 支持OpenAMP混合部署

OpenAMP是一个开源软件框架，旨在通过非对称多处理器的开源解决方案，来标准化异构嵌入式系统中操作环境之间的交互。OpenAMP包括如下四大组件：

1. remoteproc：管理从核的生命周期，管理共享内存、通信使用的buffer、vring等资源，初始化rpmsg和virtio。
2. rpmsg：实现多核通信的通道，基于virtio实现。
3. virtio：通过一套虚拟IO实现主从核的驱动程序通信，是一种半虚拟化技术。
4. libmetal：屏蔽操作系统实现细节，提供通用用户API访问设备，处理设备中断、内存请求。

## 支持POSIX标准接口

[UniProton支持posix标准接口](./uniproton-apis.md)

## 支持设备驱动

UniProton的驱动结构、风格与linux类似，将驱动设备文件化，即VFS系统，通过驱动注册接口，将驱动注册到文件系统中，应用层只需要通过标准系统调用，即可调用底层驱动。整个驱动框架代码适配自开源RTOS系统Nuttx的驱动模块，因此接口调用也与Nuttx基本一致。struct file_operations结构体保存设备文件操作的方法，定义在fs.h头文件中，通过register_driver接口将驱动设备挂到对应的struct inode节点中，struct inode描述了每个设备节点的位置和数据。当系统调用操作设备文件时，根据对应文件的inode就能索引到对应的函数。接口详细信息可以查看[UniProton接口说明](./uniproton-apis.md)。

## 支持Shell命令行

UniProton提供shell命令行，它能够以命令行交互的方式访问操作系统的功能或服务：它接受并解析用户输入的命令，并处理操作系统的输出结果。UniProton的shell模块代码适配自开源ROTS系统LiteOS的shell模块。因此与LiteOS一致，用户可以新增定制的命令，新增命令需重新编译烧录后才能执行。当前UniProton只支持了help命令，其他命令将在后续的版本中进行完善。Shell模块为用户提供下面几个接口。

|      接口名      |    描述     |
|      :---:      |    :--:    |
| SHELLCMD_ENTRY  | 静态注册命令 |
| osCmdReg        | 动态注册命令 |

通常静态注册命令方式一般用于注册系统常用命令，动态注册命令方式一般用于注册用户命令。静态注册命令有5个入参，动态注册命令有4个入参。下面除去第一个入参是静态注册独有的，剩余的四个入参两个注册命令是一致的。接口详细信息可以查看[UniProton接口说明](./uniproton-apis.md)

## UniProton介绍

UniProton主要目的在于为上层业务软件提供一个统一的操作系统平台，屏蔽底层硬件差异，并提供强大的调试功能。使得业务软件可在不同的硬件平台之间快速移植，方便产品芯片选型，降低硬件采购成本和软件维护成本。

一、搭建UniProton开发环境
----------
#### 1、下载源码
```bash
git clone https://gitee.com/openeuler/UniProton.git
```

##### 源码目录
[源码目录介绍](./doc/design/architecture_design.md)

#### 2、创建开发工程

[hello word示例](./doc/getting_started.md)

#### 3、编译

[编译步骤](./doc/UniProton_build.md)

二、功能介绍
----------

- [任务](./doc/design/task.md)
- [中断](./doc/design/hwi.md)
- [事件](./doc/design/event.md)
- [队列](./doc/design/queue.md)
- [信号量](./doc/design/sem.md)
- [内存管理](./doc/design/mem.md)
- [软件定时器](./doc/design/timer.md)
- [异常](./doc/design/exc.md)
- [错误处理](./doc/design/err.md)
- [cpu占用率](./doc/design/cpup.md)

三、免责声明
----------
1. 当前开源版本仅支持cortex_m4芯片，默认编译脚本的安全编译选项仅支持栈保护，其他选项由用户根据需要自行添加。
2. 遵循MulanPSL2开源许可协议

四、如何贡献
----------
我们非常欢迎新贡献者加入到项目中来，也非常高兴能为新加入贡献者提供指导和帮助。在您贡献代码前，需要先签署[CLA](https://openeuler.org/en/cla.html)。

# UniProton 介绍

UniProton 是一款实时操作系统，具备极致的低时延和灵活的混合关键性部署特性，可以适用于工业控制场景，既支持微控制器 MCU，也支持算力强的多核 CPU。

一、搭建 UniProton 开发环境
----------
### 1、下载源码
```bash
git clone https://gitee.com/openeuler/UniProton.git
```

[源码目录介绍](./doc/design/architecture_design.md)

### 2、创建开发工程

[hello word示例](./doc/getting_started.md)

### 3、编译

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
1. 当前开源版本支持 cortex_m4 和 armv8 芯片，默认编译脚本的安全编译选项仅支持栈保护，其他选项由用户根据需要自行添加。
2. 遵循 MulanPSL2 开源许可协议

四、如何贡献
----------
我们非常欢迎新贡献者加入到项目中来，也非常高兴能为新加入贡献者提供指导和帮助。在您贡献代码前，需要先签署 [CLA](https://clasign.osinfra.cn/sign/Z2l0ZWUlMkZvcGVuZXVsZXI=)。

openEuler 加入方式参见：[https://www.openeuler.org/zh/community/contribution](https://www.openeuler.org/zh/community/contribution)

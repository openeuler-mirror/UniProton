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

[hello word示例](./doc/demo_guide/getting_started.md)

### 3、编译

[编译步骤](./doc/demo_guide/UniProton_build.md)

### 4、运行

[树莓派4B混合部署运行示例](./doc/demo_guide/raspi4_usage.md)

[Hi3093混合部署运行示例](./doc/demo_guide/hi3093_usage.md)

[x86_64混合部署运行示例](./doc/demo_guide/x86_64_usage.md)

[RISCV64 QEMU 运行示例](./doc/demo_guide/riscv64_usage.md)

### 5、相关文档

[UniProton用户指南](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97-%E6%A6%82%E8%BF%B0.html)

[UniProton接口说明文档](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E6%8E%A5%E5%8F%A3%E8%AF%B4%E6%98%8E.html)

[yocto构建系统介绍](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/yocto/index.html)

[oebuild容器指导](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/oebuild/index.html)

[混合部署mcs仓库](https://gitee.com/openeuler/mcs)

[git commit提交规范](https://openeuler.gitee.io/yocto-meta-openeuler/master/develop_help/commit.html)

[多OS混合关键性部署框架](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/index.html)

[qemu仿真相关](./doc/qemu/)

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
1. 当前开源版本支持 cortex_m4、armv8、x86_64、riscv64芯片，默认编译脚本的安全编译选项仅支持栈保护，其他选项由用户根据需要自行添加。
2. 遵循 MulanPSL2 开源许可协议

四、如何贡献
----------
我们非常欢迎新贡献者加入到项目中来，也非常高兴能为新加入贡献者提供指导和帮助。在您贡献代码前，需要先签署 [CLA](https://clasign.osinfra.cn/sign/Z2l0ZWUlMkZvcGVuZXVsZXI=)。

openEuler 加入方式参见：[https://www.openeuler.org/zh/community/contribution](https://www.openeuler.org/zh/community/contribution)

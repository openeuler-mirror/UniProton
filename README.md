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

[qemu-riscv64-virt 开发环境搭建](./doc/riscv_qemu_start.md)

[qemu-riscv64-virt 工程创建示例](./doc/creat_app_in_riscv_qemu.md)

### 3、编译

[编译步骤](./doc/UniProton_build.md)

### 4、运行

[树莓派4B混合部署示例](./doc/demoUsageGuide/raspi4_demo_usage_guide.md)

[Hi3093混合部署示例](./doc/demoUsageGuide/hi3093_demo_usage_guide.md)

[x86_64混合部署示例](./doc/demoUsageGuide/x86_64_demo_usage_guide.md)

[UniProton gdb-stub使用指南](./doc/gdbstub.md)

[RISCV64_QEMU_HelloWorld工程](./doc/demoUsageGuide/riscv64_demo_usage_guide.md)

### 5、相关文档

[UniProton用户指南](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97-%E6%A6%82%E8%BF%B0.html)

[UniProton接口说明文档](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E6%8E%A5%E5%8F%A3%E8%AF%B4%E6%98%8E.html)

[yocto构建系统介绍](https://openeuler.gitee.io/yocto-meta-openeuler/master/yocto/index.html)

[oebuild容器指导](https://openeuler.gitee.io/yocto-meta-openeuler/master/oebuild/intro.html)

[混合部署mcs仓库](https://gitee.com/openeuler/mcs)

[git commit提交规范](https://openeuler.gitee.io/yocto-meta-openeuler/master/develop_help/commit.html)

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
1. 当前开源版本支持 cortex_m4、armv8、x86_64 芯片，默认编译脚本的安全编译选项仅支持栈保护，其他选项由用户根据需要自行添加。
2. 遵循 MulanPSL2 开源许可协议

四、如何贡献
----------
我们非常欢迎新贡献者加入到项目中来，也非常高兴能为新加入贡献者提供指导和帮助。在您贡献代码前，需要先签署 [CLA](https://clasign.osinfra.cn/sign/Z2l0ZWUlMkZvcGVuZXVsZXI=)。

openEuler 加入方式参见：[https://www.openeuler.org/zh/community/contribution](https://www.openeuler.org/zh/community/contribution)

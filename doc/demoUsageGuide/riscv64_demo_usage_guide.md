# 开发环境说明

- 开发平台：qemu-system-riscv64
- 芯片型号： virt
- OS版本信息：UniProton 24.03-LTS

## 实现过程

以 rv64virtdemo 为例：

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](../UniProton_build.md) 准备编译环境以及 libboundscheck 库下载。
4. 安装qemu-system-riscv64  红帽子系列下使用  `sudo apt install qemu-system-misc` 进行安装，其他发行版按照对应的包管理命令安装
5. 配置好编译环境后，执行 python build.py rv64virt 进行编译
6. 将目录/workspace/UniProton/build/output/rv64virt/riscv64/FPGA 中的libCortexMXsec_c.lib 和 libRV64VIRT.a 复制到 demos/riscv64virt/libs目录下
7. 将 UniProton/src/include 目录下的头文件拷贝到 demos/riscv64virt/include 目录下。
8. 可以通过修改 demos/riscv64virt/config 目录下的 prt_config.c 和 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
9. demos/riscv64virt/bsp 目录下可以新增板级驱动代码，demos/riscv64virt/build 目录下配置编译构建相关内容，rv64virt.ld为链接文件，根据单板内存地址等修改适配。
10. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `bash buibasld_app.sh` 即可在同级中的out目录下生成 rv64virt.elf 可执行二进制文件。
11. 执行build目录中的./run.sh 运行示例

## riscv64 virt 示例程序使用指南

- 按照[编译要求](../../doc/UniProton_build.md)配置好编译环境

  - 交叉编译器
  - CMake
  - Python
  - .....

- 进入 `build` 目录，修改文件 `build_app.sh` 或者 执行 `bash build_app.sh 'APP名字'`

  - **目前支持的APP**

    - **hello_world** [跑一个简单的shell]
    - **task-switch** [实时性能测试-上下文切换]
    - **task-preempt** [实时性能测试-任务抢占]
    - **semaphore-shuffle** [实时性能测试 - 信号量混洗]
    - **message-latency** [实时性能测试 - 消息队列延迟]
    - **deadlock-break** [实时性能测试-死锁解除]
    - **shell ** [UniProton shell 组件]

- 运行 **bash build_app.sh 'APP名字'** 进行编译，编译完成后，会在**当前out目录** 生成对应工程的ELF文件 和 BIN文件

- 使用**qemu**体验工程

  - ```shell
    qemu-system-riscv64 -bios none -M virt -m 512M -nographic -kernel [对应的elf文件] -smp 1
    ```

- **请注意:**

  - qemu中参数设置正确 , 目前还尚未支持S模式 所以加上 `-bios none` 同时内存需要 `>= 512M`  , 同时在链接脚本中修改参数让内存池将多余的内存进行管理
  - **如果需要自己修改参数，请配合 ld 链接脚本进行修改** 

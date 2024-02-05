# 开发环境说明

- 开发平台：qemu-system-riscv64
- 芯片型号： virt
- OS版本信息：UniProton 22.03

## 实现过程

以 rv64virtdemo 为例：

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](./UniProton_build.md) 准备编译环境以及 libboundscheck 库下载。
4. 安装qemu-system-riscv64  红帽子系列下使用  `sudo apt install qemu-system-misc` 进行安装，其他发行版按照对应的包管理命令安装
5. 配置好编译环境后，执行 python build.py rv64virt 进行编译
6. 将目录/workspace/UniProton/build/output/rv64virt/riscv64/FPGA 中的libCortexMXsec_c.lib 和 libRV64VIRT.a 复制到 demos/riscv64virt/libs目录下
7. 将 UniProton/src/include 目录下的头文件拷贝到 demos/riscv64virt/include 目录下。
8. 可以通过修改 demos/riscv64virt/config 目录下的 prt_config.c 和 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
9. demos/riscv64virt/bsp 目录下可以新增板级驱动代码，demos/riscv64virt/build 目录下配置编译构建相关内容，rv64virt.ld为链接文件，根据单板内存地址等修改适配。
10. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `bash buibasld_app.sh` 即可在同级中的out目录下生成 rv64virt.elf 可执行二进制文件。
11. 执行build目录中的./run.sh 运行示例

## riscv64 virt 示例程序

1. 配置好编译环境
2. 进入 /workspace/demos/riscv64virt/build
3. 执行 `bash build_app.sh` 生成 elf 文件 和bin 文件
4. 请确保安装了 qemu-system-riscv64
5. 执行`bash run.sh` 运行示例程序
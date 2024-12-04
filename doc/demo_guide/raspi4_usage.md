# 开发环境说明
- 开发平台：armv8
- 芯片型号：bcm2711
- OS版本信息：UniProton 24.03-LTS
- 集成开发环境：UniProton-docker

## 实现过程

以 raspi4 demo 为例：[demo目录结构](../../demos/raspi4/readme.txt)

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](./UniProton_build.md) 准备编译环境准备，推荐直接使用docker镜像编译。
4. 修改 demos/m4/config 目录下prt_config.c 以及 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
5. demos/raspi4/bsp 目录下可以新增板级驱动代码，demos/raspi4/build 目录下配置编译构建相关内容，raspi4.ld 为链接文件，根据单板内存地址等修改适配。
6. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `sh build_app.sh` 即可在同级目录下生成 raspi4 可执行二进制文件。
7. 加载到单板上运行可执行文件 raspi4。

## raspi4 示例程序
raspi4 示例程序在 demos/raspi4/apps/openamp 目录下，main.c 文件主要用于系统初始化。

## openamp 使用指导 & 结果验证

**材料准备**

1. 上述编译获取到的 raspi4 可执行二进制文件
2. 刷写好对应 openEuler 镜像的树莓派单板（具体刷写过程参考 <https://gitee.com/openeuler/raspberry>）

**使用指导**

1. 参考[多OS混合关键性部署框架](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/index.html) 配置环境并运行

   

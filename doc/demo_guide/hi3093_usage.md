# 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton 24.03-LTS
- 集成开发环境：UniProton-docker

## 实现过程

以 hi3093 demo 为例：[demo目录结构](../../demos/hi3093/readme.txt)

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](./UniProton_build.md) 准备编译环境准备，推荐直接使用docker镜像编译。
4. 可以通过修改 demos/hi3093/config 目录下的 prt_config.c 和 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
5. demos/hi3093/bsp 目录下可以新增板级驱动代码，demos/hi3093/build 目录下配置编译构建相关内容，hi3093.ld 为链接文件，根据单板内存地址等修改适配。
6. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `sh build_app.sh` 即可在同级目录下生成 hi3093 可执行二进制文件。
7. 加载到单板上运行可执行文件 hi3093。

## hi3093 示例程序
hi3093 示例程序在 demos/hi3093/apps/openamp 目录下，main.c 文件主要用于系统初始化。

## openamp 使用指导 & 结果验证

**材料准备**

1. 上述编译获取到的 hi3093 可执行二进制文件
2. 刷写好对应 openEuler 镜像的hi3093单板

**使用指导**

1. 参考[多OS混合关键性部署框架](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/index.html) 配置环境并运行

   

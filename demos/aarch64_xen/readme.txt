AArch64 Xen + standalone libmica
==============================

UniProton Xen demo，使用独立 libmica 提供 RPMSG TTY 和 shell 通信。
OS 适配接口位于 src/component/libmica_adapter，Xen 板级配置位于 bsp/ 和 config/。

构建：
  cd demos/aarch64_xen/build
  bash build_app.sh

生成 aarch64_xen.elf、aarch64_xen.bin，供 MICA 主端加载。
libmica 默认取自官方 MCS 固定版本；可用 MCS_SOURCE_DIR 指定本地源码。
配置启用 CONFIG_OS_OPTION_LIBMICA，与旧 CONFIG_OS_OPTION_OPENAMP 互斥。
运行要求：单核、MMU/cache 关闭，BSP 提供可用的共享内存 GPA 视图。

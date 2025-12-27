# Uniproton SOEM使用指南

## SOEM基本介绍
SOEM 是 Simple Open EtherCAT Master Library 的缩写，是瑞典 rt-lab 提供的一个开源 EtherCAT 主站协议库。SOEM 库使用 C 语言编写，可以在 Windows 和 Linux 平台上运行，并且可以方便地移植到嵌入式平台上。

## 开启并使用SOEM：
1. `defconfig`中开启选项 `CONFIG_OS_SUPPORT_SOEM=y`；
2. 下载SOEM归档源码至正确文件夹，并使用`UniProton-patch-for-soem.patch`补丁，参考`demos/sd3403/build/build_fetch.sh`；
3. 参考测试用例`testsuites/soemTest/soem_demo.c`，ec_init选择linux侧想要启用的网口，后续正常使用soem功能；

| 支持的平台 |
| ---------- |
| ascend310B |
| hi3093     |
| hi3095     |
| kp920_lite |
| sd3403     |

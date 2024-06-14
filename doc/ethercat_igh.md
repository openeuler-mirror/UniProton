# UniProton IgH EtherCAT使用指南

目前只有x86_64支持使用IgH EtherCAT功能，支持仅限单主站，单网口。其他基本功能相当于使用IgH EtherCAT的默认配置选项（默认配置选项参考IgH官方文档章节9.2）并关闭eoe以及rt-syslog功能，目前不支持修改IgH EtherCAT的配置选项。UniProton所支持的IgH EtherCAT接口以及结构体为用户态的一套接口，具体可参考`ecrt.h`。

UniProton支持IgH命令行工具，需要按照下文专门编译linux侧命令行工具，拉起UniProton后可以在linux侧使用该命令行工具。UniProton目前不支持linux侧通过`/dev/EtherCATx`来使用IgH功能。



## IgH EtherCAT基本功能

UniProton侧实现IgH EtherCAT功能主要是通过移植IgH EtherCAT的`master`, `lib`文件夹下的源码，以及`devices/generic.c`的源码。UniProton在`src/osal/linux`中实现了IgH EtherCAT依赖的部分内核态接口，同时通过`src/net/UniProton-patch-for-IgH.patch`文件对源码使用的部分接口进行裁剪与替换（如ioctl接口，收发包相关接口）。现阶段只支持使用i210网卡，但用户也可以自行实现其他网卡的驱动，并注册网卡钩子。

### 开启并使用IgH EtherCAT基本功能：

1. `defconfig`中开启选项 `CONFIG_OS_SUPPORT_IgH_EtherCAT=y`, `CONFIG_OS_OPTION_LINUX=y`；
2. 下载IgH归档源码至正确文件夹，并使用`UniProton-patch-for-IgH.patch`补丁，参考`demos/x86_64/build/build_fetch.sh`；
3. 代码中，使用IgH EtherCAT功能前，需要先调用`ecrt_nic_reg`注册网卡钩子函数，然后调用`ethercat_init`初始化模块。注册网卡钩子可以参考`demos/x86_64/apps/ethercat/main.c`中所使用的`ecrt_i210_nic_reg`函数；
4. 初始化IgH EtherCAT模块之后，就可以正常使用ecrt接口了，但此时背景任务可能还在自动扫描从站，需要等待从站扫描完毕。用户可以调用`wait_for_slave_respond`以及`wait_for_slave_scan_complete`工具函数等待扫描结束，或者参考这两个函数自行使用ecrt接口实现类似的功能；
5. 后续可正常使用ecrt接口；



## IgH EtherCAT命令行

UniProton对IgH命令行工具的实现主要通过三个部分组成：

1. linux侧命令行工具，基于IgH EtherCAT的`tool`文件夹下的c++源码，并替换ioctl相关接口为进程间通信接口；
2. linux侧mcs的rpmsg组件，(源码)[https://gitee.com/openeuler/mcs/tree/UniProton_dev/]，与命令行工具建立进程间通信，与UniProton通过共享内存通信，负责UniProton与命令行工具之间的消息转发；
3. UniProton侧rpmsg组件，接受命令行工具发送的请求消息，并调用对应的IgH EtherCAT接口，回复结果；

UniProton侧rpmsg组件需要开启`defconfig`中的`CONFIG_OS_OPTION_OPENAMP`选项，并添加部分rpmsg初始化代码，具体可参考`demos/x86_64`中`OS_OPTION_OPENAMP`相关代码。linux侧的mcs的编译与使用，参考(mcs源码仓)[https://gitee.com/openeuler/mcs/tree/UniProton_dev/]中的指南。

### linux侧命令行工具编译与使用：
1. 配置编译环境，参考[基于SDK的应用开发](https://openeuler.gitee.io/yocto-meta-openeuler/master/getting_started/index.html#sdk)配置x86_64编译环境，可以和mcs使用相同的编译环境。
2. 下载IgH EtherCAT[归档代码](https://gitee.com/openeuler/oee_archive/tree/master/igh-ethercat)，并使用`UniProton-patch-for-IgH.patch`补丁
3. 运行`./build.sh`编译，可执行文件`ethercat`生成在`build`文件夹内。
4. 可执行文件`ethercat`复制到linux侧的`/usr/local/bin`文件夹中, chmod +x添加执行权限, 使用mcs混合部署框架拉起UniProton后，命令行`ethercat`即可正常使用。

本目录下放置的是UniProton实时部分版本。

├─ap_boot         # 用于编译启动程序
├─apps            # 基于UniProton实时OS编程的demo程序；     
│  └─ethercat     # openamp, ethercat示例程序；
├─bsp             # 提供的板级驱动与OS对接；
├─build           # 提供编译脚本编译出最终镜像；
├─component       # 适配openamp框架patch文件，openamp源码下载存放路径;
├─config          # 配置选项，供用户调整运行时参数；
├─cplusplus       # c++基础功能支持
├─include         # UniProton实时部分提供的编程接口API；
└─libs            # UniProton实时部分的静态库，build目录中的makefile示例已经将头文件和静态库的引用准备好，应用可直接使用；

## Ethercat支持
如果需要支持Ethercat，需要在build/uniproton_config/config_x86_64/defconfig中使能CONFIG_OS_SUPPORT_IGH_ETHERCAT

## C++支持
如果需要支持C++，需要在build/uniproton_config/config_x86_64/defconfig中使能CONFIG_OS_SUPPORT_CXX
并且需要给demos/x86_64/build/build_app.sh 27行的cmake命令添加选项：-DCONFIG_OS_SUPPORT_CXX=on

## 代理接口支持
如果需要支持代理接口，需要在build/uniproton_config/config_x86_64/defconfig中使能CONFIG_OS_OPTION_PROXY

## libxml2支持
如果需要支持代理接口，需要在build/uniproton_config/config_x86_64/defconfig中使能CONFIG_OS_SUPPORT_LIBXML2

## shell支持
如果需要支持pty shell，需要在build/uniproton_config/config_x86_64/defconfig中使能CONFIG_LOSCFG_SHELL以及CONFIG_LOSCFG_SHELL_MICA_INPUT
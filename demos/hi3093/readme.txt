本目录下放置的是UniProton实时部分版本。

├─apps            # 基于UniProton实时OS编程的demo程序；     
│  └─openamp      # openamp示例程序；
├─bsp             # 提供的板级驱动与OS对接；
├─build           # 提供编译脚本编译出最终镜像；
├─component       # 适配openamp框架patch文件，openamp源码下载存放路径;
├─config          # 配置选项，供用户调整运行时参数；
├─drivers         # 适配网口和串口驱动的相关代码，以及libdrivers.a库；
├─include         # UniProton实时部分提供的编程接口API；
└─libs            # UniProton实时部分的静态库，build目录中的makefile示例已经将头文件和静态库的引用准备好，应用可直接使用；

## SOEM支持
如果需要支持SOEM，需要在build/uniproton_config/config_armv8_hi3093/defconfig中使能CONFIG_OS_SUPPORT_SOEM

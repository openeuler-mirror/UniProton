本目录下放置的是UniProton实时部分版本。

├─apps            # 基于UniProton实时OS编程的demo程序；     
│  └─hello_world  # hello_world示例程序；
├─bsp             # 提供的板级驱动与OS对接；
├─build           # 提供编译脚本编译出最终镜像；
├─config          # 配置选项，供用户调整运行时参数；
├─include         # UniProton实时部分提供的编程接口API；
└─libs            # UniProton实时部分的静态库，build目录中的makefile示例已经将头文件和静态库的引用准备好，应用可直接使用；

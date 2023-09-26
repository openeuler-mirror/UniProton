本目录下放置的是UniProton测试工程。

├─bsp             # 提供的板级驱动与OS对接；
├─build           # 提供编译脚本编译出最终镜像；
├─config          # 配置选项，供用户调整运行时参数；
├─posixtestsuite  # 适配UniProton的posix test suites；
├─rhealstone      # 适配UniProton的rhealstone性能测试；
├─include         # UniProton实时部分提供的编程接口API；
└─libs            # UniProton实时部分的静态库，build目录中的makefile示例已经将头文件和静态库的引用准备好，应用可直接使用；

qemu使用:
cd build
sh build_app.sh sim 
qemu-system-arm -M mps2-an386 -cpu cortex-m4 --semihosting -kernel UniPorton_test_xxx.bin 

注意: qemu不支持rhealstone测试, 默认不编译

调试:
qemu-system-arm -M mps2-an386 -cpu cortex-m4 --semihosting -kernel Uniproton_xxx_test.bin -s -S
gdb Uniproton_xxx_test.elf
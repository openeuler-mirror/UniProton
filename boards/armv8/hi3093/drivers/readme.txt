本目录下放置的是网口和串口驱动的相关代码。
build目录中的makefile示例已经将该文件夹下的头文件和静态库的引用准备好，应用可直接使用；

├─include         # hi3093网口和串口驱动的编程接口API，/demos/hi3093/CMakeLists.txt中将其添加到头文件搜索路径；
├─libs            # hi3093网口和串口驱动的静态库，/demos/hi3093/CMakeLists.txt中会链接libdriver.a静态库；
└─samples         # 网口和串口驱动的测试用例；

## net sample
1. 编译带网口测试用例的二进制文件
   （1）cd /demos/hi3093/build 
   （2）sh build_app.sh netTest

2. 使用
   （1）隔离出网口ETH0
   （2）网口ETH0连接网线
   （3）拉起UniProton
   （4）观察串口uart2打印，是否收到消息

3. 网口收发消息
   （1）发消息代码参考sample_net.c中的接口sample_net_send
   （2）收消息代码参考sample_net.c中的接口sample_net_receive

## uart sample
1. 编译带串口测试用例的二进制文件
   （1）cd /demos/hi3093/build 
   （2）sh build_app.sh uartTest

2. 使用
   （1）隔离出串口uart4
   （2）拉起UniProton
   （3）在uart4输入字符
   （4）观察串口uart2打印，是否和输入字符相符

3. 串口收发消息以及串口打印
   （1）发消息代码参考sample_uart.c中的接口sample_uart_send
   （2）收消息代码参考sample_net.c中的接口sample_uart_interrupt_receive，注意中断号必须为92
   （3）串口打印代码参考sample_common.c中的接口sample_prepare，之后需要打印的时候使用bm_printf接口即可


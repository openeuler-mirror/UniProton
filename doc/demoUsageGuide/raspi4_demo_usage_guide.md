# 开发环境说明
- 开发平台：armv8
- 芯片型号：bcm2711
- OS版本信息：UniProton 22.03
- 集成开发环境：UniProton-docker

## 实现过程

以 raspi4 demo 为例：[demo目录结构](../demos/raspi4/readme.txt)

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](./UniProton_build.md) 准备编译环境以及 libboundscheck 库下载。
4. 编译生成的 libRASPI4.a 文件和 libCortexMXsec_c.lib 文件在 UniProton/output/UniProton/lib/raspi4/ 目录下。将这两个静态库文件拷贝到 demos/raspi4/libs 目录下。
5. 将 UniProton/src/include 目录下的头文件拷贝到 demos/raspi4/include 目录下。
6. 可以通过修改 demos/raspi4/config 目录下的 prt_config.c 和 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
7. demos/raspi4/bsp 目录下可以新增板级驱动代码，demos/raspi4/build 目录下配置编译构建相关内容，raspi4.ld 为链接文件，根据单板内存地址等修改适配。
8. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `sh build_app.sh` 即可在同级目录下生成 raspi4 可执行二进制文件。
9. 加载到单板上运行可执行文件 raspi4。

## raspi4 示例程序
raspi4 示例程序在 demos/raspi4/apps/openamp 目录下，main.c 文件主要用于系统初始化，rpmsg_backend.c 文件主要用于 openamp 后端资源的初始化，rpmsg_service.c 文件主要用于 openamp 的前端应用。

## openamp 使用指导 & 结果验证

**材料准备**

1. 上述编译获取到的 raspi4 可执行二进制文件
2. 对应版本的 mcs_km.ko、rpmsg_main，libmetal、libopen_amp、libsysfs 的动态库（具体构建过程参考 <https://gitee.com/openeuler/mcs>）
3. 刷写好对应 openEuler 镜像的树莓派单板（具体刷写过程参考 <https://gitee.com/openeuler/raspberry>）

**使用指导**

1. 将上述1和2的材料通过ssh传输到树莓派单板上，这里假设传到 /root/mcsdir 目录。

2. 通过 `chmod +x rpmsg_main` 添加可执行权限

3. 将 libmetal.so\*、libopen_amp.so\* 复制到 /usr/lib64 目录下，将 libsysfs.so 复制到 /lib64 目录下

4. 进入 /root/mcsdir 目录依次执行：

   ```bash
   insmod mcs_km.ko
       
   # 执行rpmsg_main，
   # -c cpu           指定在哪个cpu拉起二进制文件
   # -t raspi4.bin    可执行二进制文件
   # -a address       指定二进制文件的加载地址
   ./rpmsg_main -c 3 -t raspi4.bin -a 0x7a000000
   ```

5. 运行后，根据提示使用对应的 pty 设备和 UniProton 进行通信：

   ```bash
   raspberrypi4-64 ~ # insmod mcs_km.ko
   raspberrypi4-64 ~ # ./rpmsg_main -c 3 -t raspi4.bin -a 0x7a000000
   cpu:3, ld:7a000000, entry:7a000000, path:raspi4.bin
   mcs fd:3
   
   Initialize the virtio, virtqueue and rpmsg device
   virt add:0x7f8b700000, status_reg:0x7f8b700000, tx:0x7f8b72c000, rx:0x7f8b728000, mempool:0x7f8b704000
   number of services: 1
   start client os
   pty master fd is :4
   pls open /dev/pts/1 to talk with client OS
   pty_thread for uart is runnning
   pty master fd is :5
   pls open /dev/pts/2 to talk with client OS
   pty_thread for console is runnning
   found matched endpoint, creating console with id:2 in host os
   ```

   如上所示，接着在另一个 ssh 窗口中，运行 `screen /dev/pts/2`，此处在该窗口中，敲击回车即可触发 demo 程序。

   可以在 screen 界面上看到输出结果如下：

   ```bash
   hello, UniProton!
   ```

   

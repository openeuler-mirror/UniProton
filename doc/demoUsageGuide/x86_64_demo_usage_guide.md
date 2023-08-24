## 前置条件
原生 ubuntu 22.04.1

## oebuild安装
参考 "运行环境准备"，安装python3 pip oebuild

https://openeuler.gitee.io/yocto-meta-openeuler/master/yocto/oebuild.html#id2

参考 "如何编译一个标准镜像"，尝试构建默认镜像（操作时需要切换到普通用户权限）

https://openeuler.gitee.io/yocto-meta-openeuler/master/yocto/oebuild.html#id4

例子：
```sh
mkdir example
cd ./example
oebuild init tmp
cd ./tmp 
oebuild update
oebuild generate
cd ./build/aarch64-std
oebuild bitbake openeuler-image
```

结果显示没有error messages，代表构建默认镜像成功，编辑环境正常

## 编译 uniproton
在 build/aarch64-std 文件夹内，使用 oebuild bitbake, 进入编译环境
下载并编译 uniproton 个人分支代码:

```sh
git clone https://gitee.com/openeuler/UniProton.git
cd UniProton
git checkout -b dev origin/dev
cd demos/x86_64/build
sh ./build_app.sh
```

镜像路径: build/aarch64-std/UniProton/demos/x86_64/build/x86_64.bin

## 编译 mcs_km.ko及rpmsg_main
参考 "构建安装指导"，构建出mcs_km.ko及rpmsg_main，并根据指导拷贝libmetal,libopen_amp,libsysfs到安装环境
https://gitee.com/openeuler/mcs/tree/uniproton_dev/ （注意要使用uniproton_dev分支）

可在当前root用户目录下创建deploy目录，把生成的mcs_km.ko,rpmsg_main，及前面生成的x86_64.bin统一放到此目录

## 编译 ap_boot
```sh
cd demos/x86_64/ap_boot
make
```
把当前目录下生成的ap_boot文件拷贝到deploy目录

## 预留CPU及内存
按需给uniproton预留要使用的CPU及内存，如四核CPU可预留一核，16G物理内存可预留4G。可通过修改boot分区的grub.cfg配置内核启动参数，新增maxcpus及mem参数，参考如下：
```sh
openEuler-Embedded ~ # mount /dev/sda1 /boot
openEuler-Embedded ~ # cat /boot/efi/boot/grub.cfg
# Automatically created by OE
serial --unit=0 --speed=115200 --word=8 --parity=no --stop=1
default=boot
timeout=10

menuentry 'boot'{
linux /bzImage  root=*** rw rootwait quiet  console=ttyS0,115200 console=tty0 maxcpus=3 mem=12G
}
```

## 部署 uniproton
```sh
#插入ko
# load_addr        指定二进制文件的加载地址
# 对8G内存环境加载地址为0x1c0000000
insmod mcs_km.ko load_addr=0x1c0000000
# 对16G内存环境加载地址为0x400000000
insmod mcs_km.ko load_addr=0x400000000

# 执行rpmsg_main
# -c cpu           指定在哪个cpu拉起二进制文件
# -t x86_64.bin    可执行二进制文件
# -a address       指定二进制文件的加载地址
# 对8G内存环境加载地址为0x1c0000000
./rpmsg_main -c 3 -t x86_64.bin -a 0x1c0000000
# 对16G内存环境加载地址为0x400000000
./rpmsg_main -c 3 -t x86_64.bin -a 0x400000000
```

运行后，根据提示使用对应的pty设备和UniProton进行通信：
```sh
openEuler-Embedded ~ # cd ~/deploy
openEuler-Embedded ~/deploy # insmod mcs_km.ko load_addr=0x1c0000000
openEuler-Embedded ~/deploy # ./rpmsg_main -c 3 -b ap_boot -t x86_64.bin -a 0x1c0000000
cpu:3, ld:1c0000000, entry:1c0000000, path:x86_64.bin share_mem:1bde00000

Initialize the virtio, virtqueue and rpmsg device
virt add:0x7f98776d3000, status_reg:0x7f98776d3000, tx:0x7f98776ff000, rx:0x7f98776fb000, mempool:0x7f98776d7000
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
如上所示，接着在另一个ssh窗口中，运行screen /dev/pts/2(screen打开的窗口需要和上面第二次出现的/dev/pts/id中的id对应），在该窗口中，敲击回车即可触发demo程序。

可以在screen界面上看到输出结果如下：
```
hello, UniProton!
```
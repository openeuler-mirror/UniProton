## 前置条件
原生 ubuntu 22.04

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
把当前目录下生成的ap_boot文件拷贝到运行环境的/lib/firmware目录下

## 预留CPU及内存
按需给uniproton预留要使用的CPU及内存，如四核CPU建议预留一个核，内存建议预留256M， 可通过修改boot分区的grub.cfg配置内核启动参数，新增 maxcpus=3 memmap=256M\$0x110000000 参数，参考如下：
```sh
openEuler-Embedded ~ # mount /dev/sda1 /boot
openEuler-Embedded ~ # cat /boot/efi/boot/grub.cfg
# Automatically created by OE
serial --unit=0 --speed=115200 --word=8 --parity=no --stop=1
default=boot
timeout=10

menuentry 'boot'{
linux /bzImage  root=*** rw rootwait quiet maxcpus=3 memmap=256M\$0x110000000 console=ttyS0,115200 console=tty0
}
```

## 部署 uniproton
  - 启动 UniProton：
     运行 `mica start x86_64.bin`
  - 为Uniproton分配第id个I210网卡，并启动Uniproton（默认最后一张id为-1）：
     运行 `mica start x86_64.bin [-x id]`
  - 停止 UniProton：
     运行 `mica stop`
mica命令中封装了所需要的ko以及ko加载参数，如无定制诉求，使用默认即可。

运行后，根据提示使用对应的pty设备和UniProton进行通信：
```sh
openEuler-Embedded ~ # cd ~/deploy
openEuler-Embedded ~/deploy # mica start x86_64.bin
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
# 使用qemu实现x86-64混合部署

以宿主机系统Ubuntu为例，操作流程如下

### 1、虚拟机环境准备

首先确认宿主机是否支持虚拟化，对英特尔处理器执行如下命令有VT-x输出即可
```bash
LC_ALL=C lscpu | grep Virtualization
```
另外需要确保宿主机上有多个处理器核心，执行如下命令输出大于零即可
```bash
egrep -c '(vmx|svm)' /proc/cpuinfo
```
安装qemu-kvm
```bash
sudo apt-get install qemu-kvm libvirt-bin bridge-utils virt-manager
```
将用户添加到组，启用并启动libvirt服务
```bash
sudo useradd -g $USER libvirt
sudo useradd -g $USER libvirt-kvm
sudo systemctl enable libvirtd.service && sudo systemctl start libvirtd.service
```

### 2、安装qemu

下载最新版本的[qemu源码](https://www.qemu.org/download/)，以当前8.1.2版本为例，依次执行如下操作编译并安装qemu
```bash
wget https://download.qemu.org/qemu-8.1.2.tar.xz
tar xvJf qemu-8.1.2.tar.xz
cd qemu-8.1.2
sudo ./configure
sudo make -j16
sudo make install
```
执行如下命令查看版本正确说明安装成功
```bash
qemu-system-x86_64 --version
```

### 3、安装openEuler Embedded

参考[安装说明](https://openeuler.gitee.io/yocto-meta-openeuler/master/bsp/x86/qemu/qemu.html)，安装openEuler Embedded（使用mcs版本）

[镜像路径](http://121.36.84.172/dailybuild/EBS-openEuler-23.09/EBS-openEuler-23.09/embedded_img/x86-64/x86-64-mcs/)

### 4、配置虚拟机核心数、内存，并为UniProton预留

虚拟机配置参考如下，内存为8G、核心数为4
```bash
sudo qemu-system-x86_64 -enable-kvm -cpu host -m 8G -nographic -bios OVMF.fd -hda disk.img -smp 4,cores=1,threads=4,sockets=1
```
在GRUB.cfg中设置预留为：maxcpus=3 memmap=256M\$0x110000000
配置好后关闭openEuler Embedded

### 5、配置宿主机及虚拟机网络

宿主机创建配置如下：
```bash
ip tuntap add dev tap0 mode tap
ip link set dev tap0 up
ip address add dev tap0 192.168.2.128/24
```
以如下命令启动openEuler Embedded
```bash
sudo qemu-system-x86_64 -enable-kvm -cpu host -m 8G -nographic -bios OVMF.fd -hda disk.img -smp 4,cores=1,threads=4,sockets=1 -net nic -net tap,ifname=tap0,script=no,downscript=no
```
启动后在openEuler Embedded配置如下ip，enp0s3是openEuler Embedded里的网卡，根据实际网卡名修改即可
```bash
ip addr add 192.168.2.129/24 dev enp0s3
ip link set enp0s3 up
ping 192.168.2.128 -c 4
```
如上能ping通说明网络已打通，即可通过scp传输文件、ssh远程连接

### 6、在openEuler Embedded里拉起UniProton

首先在宿主机中执行
```bash
cat /proc/cpuinfo | grep x2apic
```
查看是否支持x2apic，如果不支持，上一步中qemu启动参数-cpu host修改为-cpu max即可

新装的openEuler Embedded系统是没有/lib/firmware目录的，需要创建并从宿主机传入ap_boot
```bash
mkdir -p /lib/firmware
```
在宿主机拷贝ap_boot、x86_64.bin到openEuler Embedded，拉起UniProton，参考如下：
```bash
scp ap_boot root@192.168.2.129:/lib/firmware
scp x86_64.bin root@192.168.2.129:/root
mica start x86_64.bin
```
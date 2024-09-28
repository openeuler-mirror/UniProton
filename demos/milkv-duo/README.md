## 使用指南

## 下载工具链

- MilkV-Duo 开发板使用的工具链是官方提供的工具链 【 UniProton 内核编译和 demo 编译均使用的是同一套工具链 】 

- 工具链地址 ： https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/07/16/host-tools.tar.gz

- 下载完成后使用

- ```shell
  sudo tar -xvf  path2host-tools.tar.gz -C /opt/buildtools/
  ```

- 如果不存在 `/opt/buildtools` 目录 ， 创建即可

- 其他的 UniProton 使用的工具链根据 UniProton 的文档自行配置即可！

## 编译出小核镜像

- 配置好工具链和 UniProton 其他需要的工具后, 在 build 目录下 执行

- ```shell
  bash build.sh 
  ```

- 进行编译即可

- 得到的结果会输出到

- ```shell
  build/output/openamp.elf
  ```

- 将 openamp.elf 传递到 MilkV-Duo 大核Linux 的目录进行烧录即可！

- 对应目录为 `/lib/firmware/milkvduo_uniproton.elf!!!`





# openEuler RISC-V 上使用 MICA 对 MilkV-Duo 小核管理

## 演示视频效果

由于 gif 图片较大，无法放下，在链接里面可以看到
 [https://github.com/openeuler-riscv/duo-buildroot-sdk](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Fopeneuler-riscv%2Fduo-buildroot-sdk)

## 项目介绍

MICA

- 介绍 : 在一颗片上系统中部署多个OS，同时提供Linux的服务管理能力以及实时OS带来的高实时、高可靠的关键能力。
- 仓库链接 : https://gitee.com/openeuler/mcs
- 在 MilkV-Duo 上适配修改过的仓库链接 : https://gitee.com/Jer6y/mcs
- 作用 : 在 Linux 上 提供管理小核生命周期以及和小核交流

UniProton

- 介绍 : UniProton 是一款实时操作系统，具备极致的低时延和灵活的混合关键性部署特性，可以适用于工业控制场景，既支持微控制器 MCU，也支持算力强的多核 CPU。
- 仓库链接 : https://gitee.com/openeuler/UniProton
- 在 MilkV-Duo 上适配修改过的仓库链接 : https://gitee.com/Jer6y/UniProton
- 作用 : 在 MIlkV-Duo 小核上运行，并和 MilkV-Duo 大核 Linux 交流通信

## SD 卡镜像烧录

### 1. 镜像烧录，开箱即用

我事先准备好了可以直接烧录的镜像，镜像里面跑的是 大核 openEuler RISC-V 2303 ， 小核 UniProton , 可以直接使用 mica 对小核进行管理，也可以对小核镜像进行热更新

需要准备的硬件

- MilkV-Duo 硬件 (64M)
- 至少 16G 的 SD卡
- 读卡器

需要准备的软件

- Ubuntu 或其他发行版
- [oe2303 + uniproton 镜像](https://gitee.com/link?target=https%3A%2F%2Fshare.weiyun.com%2F0gIkzesF)

烧录流程

1. 根据提供的链接，下载镜像
2. 使用 `gunzip path2img/oe2303_uniproton.img.gz` 解压镜像，得到 `oe2303_uniproton.img`
3. 将 SD 卡插入读卡器一端，再将读卡器插入到电脑的 USB 卡槽  [此处电脑是 Ubuntu 或 其他发行版]
4. 使用 `lsblk` 查看是否正确识别到 SD 卡 , 比如 `/dev/sda`
5. 使用 `sudo dd if=oe2303_uniproton.img of=/dev/sda bs=1M status=progress` 烧录到 SD 卡中
6. 烧录成功后，直接插入 MilkV-Duo 的 SD 卡槽，开机启动即可。
7. 登陆系统
   1. 用户名 ： root
   2. 密码 : openEuler12#$
   3. 注意 ：RNDIS可能不可用，因此建议使用UART连接。

### 2. SDK编译，自DIY

MICA 的运行并不一定需要 openEuler , 只要能够解决 mica 的软件依赖  libsysfs, libmetal, libopenamp 即可在其他发行版上运行 MICA

此前已经有很多社区的伙伴在 MilkV-Duo 上运行过其他的发行版

[Arch Linux](https://gitee.com/link?target=https%3A%2F%2Fcommunity.milkv.io%2Ft%2Farch-linux-on-milkv-duo-milkv-duo-arch-linux%2F329)

[Debian Linux](https://gitee.com/link?target=https%3A%2F%2Fcommunity.milkv.io%2Ft%2Fdebian-arch-linux-on-milkv-duo-256m-milkv-duo-256m-debian-arch-linux%2F1110)

......

可根据上述的链接，利用 [duo-buildroot](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Fopeneuler-riscv%2Fduo-buildroot-sdk) 编译出镜像，将其他发行版跑在 MilkV-Duo 上, 然后安装 软件依赖，编译 [mcs](https://gitee.com/Jer6y/mcs) 即可使用

[注 : 需要使用链接中提到的 sdk , 里面有针对 mica 的内核驱动模块 和 控制管理小核 UniProton 的 uni_pedestal 基座]

下面给出编译烧录openEuler 2303 + UniProton 的步骤  [ 下面的步骤参考了[社区运行MilkV-Duo on openEuler](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Fruyisdk%2Fsupport-matrix%2Fblob%2Fmain%2FDuo%2FopenEuler%2FREADME.md) 的案例，在此致谢！！]

1. 下载 sdk [魔改后的版本]

   ```shell
   git clone https://github.com/openeuler-riscv/duo-buildroot-sdk.git
   cd duo-buildroot-sdk/
   ```

2. 编译烧录出镜像

   ```shell
   docker run -itd --name duodocker -v $(pwd):/home/work milkvtech/milkv-duo:latest /bin/bash
   docker exec -it duodocker /bin/bash -c "cd /home/work && cat /etc/issue && ./build.sh milkv-duo"
   ```

3. 拷贝镜像

   ```shell
   cd ..
   cp duo-buildroot-sdk/out/milkv-duo-yyyyMMdd-hhmm.img .
   ```

4. 烧录镜像

   ```shell
   sudo dd if=milkv-duo-yyyyMMdd-hhmm.img of=/dev/your-device bs=1M status=progress
   ```

5. 准备替换根文件系统。由于原始镜像的根文件系统空间不足，需要重新分区。由于我们不会使用原始的根文件系统，因此简单地删除原始分区即可。

   ```shell
   sudo fdisk /dev/your-device
   
   # In fdisk
   d
   3
   d
   2
   n
   p
   2
   
   w
   
   # In bash
   sudo mkfs.ext4 /dev/your-device-p2
   ```

6. 从[这里](https://gitee.com/link?target=https%3A%2F%2Fshare.weiyun.com%2F39hT2j4a)下载已经带有 MICA 和 UniProton 的 openEuler 2303 的根文件系统

7. 将 SD 卡的第二个分区挂到一个目录上并解压根文件系统到该目录

   ```shell
   mkdir mnt
   sudo mount /dev/your-device-p2 mnt
   sudo tar -zxvf /path/to/oe2303_uniproton_rootfs.tar.gz -C mnt
   ```

8. 取消挂载，拔出 SD 卡 ，安装到 MilkV-Duo上

   ```shell
   sudo umount mnt
   ```

9. 登陆系统

   1. 用户名 ： root
   2. 密码 : openEuler12#$
   3. 注意 ：RNDIS可能不可用，因此建议使用UART连接。

再给出在其他发行版上使用 MICA 的大体步骤:

1. 根据 社区支持 基于改动过的 [sdk](https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2Fopeneuler-riscv%2Fduo-buildroot-sdk) 成功运行 其他发行版  [其中 sdk 主要是提供 mica 需要的驱动模块，修改了相关的设备树 ]
2. 根据对应发行版的包管理工具自行下载 libsysfs, libmetal, libopenamp 或者自己根据源码编译
3. 根据 MICA README 文档编译 MICA 项目, 能够在 Duo 上适配的 MICA 项目在[这里](https://gitee.com/Jer6y/mcs)
4. 体验 MICA

## 使用 MICA 管理小核

### 小核生命周期管理，小核热插拔、热更新

1. 首先抑制内核日志的打印等级，防止内核信息打印到屏幕上 , 导致信息错乱

   ```shell
   echo "1 4 1 7" > /proc/sys/kernel/printk
   ```

2. 检查 Client OS 的状态 , 这里可以观察到 Client OS 的状态和名字

   ```shell
   mica status
   ```

3. 启动 Client OS , 可以看到 uniproton 输出的一些日志信息 [如果不想要，自己可以后续热更新小核镜像的时候去掉就好了]

   ```shell
   mica start uniproton
   ```

4. 关闭 Client OS

   ```shell
   mica stop uniproton
   ```

## 小核tty service

1. 在执行 `mica status` 的时候，可以看到 Client OS 绑定的服务 ， 比如 可能会绑定 tty service 到 `/dev/ttyRPMSG1` 设备上

2. 可以执行 screen 打开 Client OS 的 shell

   ```shell
   screen /dev/ttyRPMSG0
   ```

3. 体验 Client OS [UniProton] 的shell

## 小核 rpc service

1. 暂未支持 【 TODO 会在完成支持后对这部分文档进行完善 】

## 添加自己的 service

1. 详情请见项目 [UniProton](https://gitee.com/openeuler/UniProton) 和 项目 [MICA](https://gitee.com/openeuler/mcs) 【 TODO 会在上游仓库的 PR 合入后对这部分的文档进行完善 】

## 在线热更新小核镜像

- 小核镜像在 `/lib/firmware/milkvduo_uniproton.elf`
- 小核开发详情请见 [UniProton](https://gitee.com/openeuler/UniProton)   【 TODO 在上游支持的 PR 合入后对这部分的文档进行完善 】

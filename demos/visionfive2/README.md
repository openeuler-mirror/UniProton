# 使用指南

 UniProton 使用的工具链根据 UniProton 的文档自行配置即可！

## 编译出小核镜像

配置好工具链和 UniProton 其他需要的工具后, 在 build 目录下 执行

```shell
bash build_app.sh
```

如果编译成功，会在out目录下生成三个文件，分别是shell.asm、shell.bin、shell.elf，接下来开始制作VisionFive2 SD卡。

## 在Visionfiv2上使用Uniproton

实验开始前，你需要准备一个32G SD卡，注意SD卡的兼容性，下面这个链接中测试给出的品牌都可以选择。

https://bret.dk/best-microsd-card-for-the-visionfive-2/

准备好SD卡后，开始制作，**注意将下面命令中/dev/sdX中sdX的替换为你的设备名称**

首先，给SD卡分区：

```shell
$ sudo sgdisk -g --clear --set-alignment=1 \
--new=1:4096:+2M: --change-name=1:'spl' --typecode=1:2e54b353-1271-4842-806f-e436d6af6985 \
--new=2:8192:+16M: --change-name=2:'opensbi-uboot' --typecode=2:5b193300-fc78-40cd-8002-e86c45580b47 \
--new=3:40961:+64M --change-name=3:'efi' --typecode=3:C12A7328-F81F-11D2-BA4B-00A0C93EC93B \
/dev/sdX
$ sudo mkfs.fat -F 32 /dev/sdX3
```

接下来，在build目录下，将固件和系统映像写入SD卡：

```shell
$ sudo dd if=./fw/u-boot-spl.bin.normal.out of=/dev/sdX1
$ sudo dd if=./fw/fw.img of=/dev/sdX2
$ sudo mount /dev/sdX3 /mnt
$ sudo cp ./output/shell.bin /mnt/
$ sudo umount /mnt
```

到此，SD卡制作完毕，将SD开插入Visionfive2主板，将启动开关调节为从SD卡启动，上电后，按提示输入`shell.bin`就可以将Uniproton内核加载并执行。

**Note：其中build/fw目录下的固件`u-boot-spl.bin.normal.out`为Uboot官方固件（经过简单的修改），`fw.bin` 为自定义固件，[QIUZHILEI/vf2_bootloader: 替代VisionFive 2 Uboot，用于加载自定义操作系统内核](https://github.com/QIUZHILEI/vf2_bootloader)**

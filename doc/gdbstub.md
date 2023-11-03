# UniProton gdbstub使用指南

## 1 编译时使能gdbstub功能：
* defconfig 设置CONFIG_OS_GDB_STUB=y

## 2 如何进行调试：
混合部署场景下，需要在linux上支持转发的功能，用于在gdbstub和gdb之间转发RSP包。根据转发模块形态的不同，可分为下面2种使用方式。
### 2.1 使用mica_main进行调试
当前在mica_main中已经集成了转发的功能，在编译了支持gdbstub功能的bin后，可以直接使用mica_main进行调试。
可以参考[使用方法和示例](https://openeuler.gitee.io/yocto-meta-openeuler/master/features/mica/mica_openamp.html#gdb-stub-client-os)。

### 2.2 使用独立转发程序进行调试
可参考[转发程序](https://gitee.com/zuyiwen/stub/tree/master/agent-server)实现。
分别拉起UniProton、和转发程序后，可以通过gdb进行连接调试，如下所示：
```
openEuler-Embedded ~ # gdb uvpck.elf
GNU gdb (GDB) 12.1
Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-openeuler-linux".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from stub.elf...
(gdb) target remote :9999
Remote debugging using :9999
warning: Remote gdbserver does not support determining executable automatically.
RHEL <=6.8 and <=7.2 versions of gdbserver do not support such automatic executable detection.
The following versions of gdbserver support it:
- Upstream version of gdbserver (unsupported) 7.10 or later
- Red Hat Developer Toolset (DTS) version of gdbserver from DTS 4.0 or later (only on x86_64)
- RHEL-7.3 versions of gdbserver (on any architecture)
0x0000000f0261b38b in OsGdbArchInit () at /home/openeuler/repo/UniProton/src/component/gdbstub/arch/x86_64/gdbstub.c:211
211     /home/openeuler/repo/UniProton/src/component/gdbstub/arch/x86_64/gdbstub.c: No such file or directory.
(gdb) b OsTestInit
Breakpoint 1 at 0xf026144d0: file /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/main.c, line 52.
(gdb) b rpmsg_endpoint_cb
Breakpoint 2 at 0xf02614ca0: file /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/example_default.c, line 20.
(gdb) c
Continuing.

Breakpoint 1, OsTestInit () at /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/main.c:52
52      /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/main.c: No such file or directory.
(gdb) c
Continuing.

Breakpoint 2, rpmsg_endpoint_cb ([remote] Sending packet: $mf02c85640,40#2f
ept=0xf02a845a0 <g_ept>, data=0xf00406010, len=1, src=1024, priv=0x0) at /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/example_default.c:20
20      /home/openeuler/repo/UniProton/demos/uvpck/apps/ethercat/example_default.c: No such file or directory.
(gdb) c
Continuing.
```
## 3 ringbuffer适配指南
当前仅支持调试运行在树莓派4B（aarch64）和x86工控机（x86_64）的UniProton。
UniProton和linux侧的转发进程之间通过ringbuffer进行通讯，如果想在aarch64/x86_64其他单板上使用此功能，需要修改ringbuffer配置。
转发程序中ringbuffer的rxaddr和txaddr与UniProton相反，size相同。
例如，UniProton侧配置的虚拟地址如下：
```
    .rxaddr = 0xf02600000 - 0x3000,
    .txaddr = 0xf02600000 - 0x4000,
    .size = 0x1000
```
linux侧配置的是物理地址，假设虚拟地址0xf02600000对应的物理地址是0x400000000，
那么linux侧的ringbuffer配置应为：
```
    .rxaddr = 0x400000000 - 0x4000,
    .txaddr = 0x400000000 - 0x3000,
    .size = 0x1000
```
# 4 其他注意事项
当前的gdbstub需要对代码段进行修改，对应的页表项上需要加上额外的写权限。修改方法如下：
## aarch64 
参考demos/raspi4/bsp/mmu.c，确定有MMU_ACCESS_RWX
```c
        .virt      = MMU_IMAGE_ADDR,
        .phys      = MMU_IMAGE_ADDR,
        .size      = 0x1000000,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
```

## x86_64
使用的页表有linux侧提前初始化好，因此需要修改mcs_km的[代码](https://gitee.com/openeuler/mcs/blob/uniproton_dev/mcs_km/mmu_map.c)
```c
		// text
		.va = 0xf02600000,
		.pa = 0x0,
		.size = 0x400000,
		.attr = MEM_ATTR_CACHE_RWX,
		.page_size = PAGE_SIZE_2M,
```
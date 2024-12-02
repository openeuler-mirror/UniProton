# UniProton gdbstub使用指南

## 1 编译时使能gdbstub功能：
* defconfig 设置CONFIG_OS_GDB_STUB=y
* aarch64需要额外设置CONFIG_OS_OPTION_POWEROFF=y

## 2 如何进行调试：
混合部署场景下，需要在linux上实现转发模块，用于在gdbstub和gdb之间进行消息转发。
当前在mica中已经集成了转发的功能，在编译了支持gdbstub功能的elf后，可以直接使用mica进行调试。
可以参考[使用方法和示例](https://embedded.pages.openeuler.org/master/features/mica/instruction.html)。

注: 在x86_64版本中需要额外将gdbmgr拷到验证环境上。

## 3 gdbstub支持情况
当前支持架构为aarch64/x86_64，已经支持的demo如下：
* x86_64 (未适配资源表)
* hi3093
* raspi4
* kp920 (未适配资源表)

当前主要支持命令及功能如下：
* break
* continue
* print (不包括调用函数)
* quit
* backtrace
* run
* watch
* Ctrl+C
* delete
* finish
* step
* next
* info local/regs

## 4 适配指南
aarch64/x86_64新增demo的适配主要包括以下两步:
* 链接脚本适配：参照现有demo补充缺失符号
* ringbuffer适配：如果已经支持资源表，可跳过此步，无需进行适配

UniProton和linux侧的转发进程之间通过ringbuffer进行通讯，对于未适配资源表的场景，需要手动适配ringbuffer，具体可参考demos/x86_64/bsp/gdbstub_cfg.c。
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

## 5 其他注意事项
当前的gdbstub需要对代码段进行修改，对应的页表项上需要加上额外的写权限。修改方法如下：
### aarch64 
参考demos/raspi4/bsp/mmu.c，确定有MMU_ACCESS_RWX
```c
        .virt      = MMU_IMAGE_ADDR,
        .phys      = MMU_IMAGE_ADDR,
        .size      = 0x1000000,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
```

### x86_64
使用的页表有linux侧提前初始化好，因此需要修改mcs_km的[代码](https://gitee.com/openeuler/mcs/blob/uniproton_dev/mcs_km/mmu_map.c)
```c
		// text
		.va = 0xf02600000,
		.pa = 0x0,
		.size = 0x400000,
		.attr = MEM_ATTR_CACHE_RWX,
		.page_size = PAGE_SIZE_2M,
```
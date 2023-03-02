## 开发环境说明
开发平台：armv8

芯片型号：bcm2711

OS版本信息：UniProton 22.03

集成开发环境：UniProton-docker

### 实现过程
以RASPI4 demo为例：[demo目录结构](../demos/RASPI4/readme.txt)
<ol>
<li>进入到用户的工作目录，这里假设为/workspace/UniProton。</li>
<li>将UniProton的源码放在/workspace/UniProton目录下。</li>
<li>参考[编译指导](./UniProton_build.md) 准备编译环境以及libboundscheck库下载。</li>
<li>编译生成的libRASPI4.a文件和libCortexMXsec_c.lib文件在UniProton/output/UniProton/lib/raspi4/目录下。将这两个静态库文件拷贝到demos/RASPI4/libs目录下。</li>
<li>将UniProton/src/include目录下的头文件拷贝到demos/RASPI4/include目录下。</li>
<li>可以通过修改demos/RASPI4/config目录下的prt_config.c和prt_config.h以适配用户功能，prt_config.h可配置os功能开关，按需裁剪。</li>
<li>demos/RASPI4/bsp目录下可以新增板级驱动代码，demos/RASPI4/build目录下配置编译构建相关内容，raspi4.ld为链接文件，根据单板内存地址等修改适配。</li>
<li>代码修改完成后，适配cmake，最后在build目录下运行sh build_app.sh即可在同级目录下生成RASPI4可执行二进制文件。</li>
<li>加载到单板上运行可执行文件RASPI4。</li>
</ol>

### RASPI4 示例程序
RASPI4 示例程序在demos/RASPI4/apps/openamp目录下，main.c文件主要用于系统初始化，rpmsg_backend.c文件主要用于openamp后端资源的初始化，rpmsg_service.c文件主要用于openamp的前端应用。

### openamp使用指导 & 结果验证
**材料准备**
<li>1、上述编译获取到的RASPI4可执行二进制文件</li>
<li>2、对应版本的mcs_km.ko、rpmsg_main，libmetal、libopen_amp、libsysfs的动态库(具体构建过程参考https://gitee.com/openeuler/mcs)
<li>3、刷写好对应openEuler镜像的树莓派单板(具体刷写过程参考https://gitee.com/openeuler/raspberry)</li>
**使用指导**
<li>1、将上述1和2的材料通过ssh传输到树莓派单板上，这里假设传到/root/mcsdir目录。</li>
<li>2、通过chmod +x rpmsg_main添加可执行权限</li>
<li>3、将libmetal.so\*、libopen_amp.so\*复制到/usr/lib64目录下，将libsysfs.so复制到/lib64目录下</li>
<li>4、进入/root/mcsdir目录依次执行

    insmod mcs_km.ko
	
	# 执行rpmsg_main，
	# -c cpu           指定在哪个cpu拉起二进制文件
	# -t RASPI4.bin    可执行二进制文件
	# -a address       指定二进制文件的加载地址
	./rpmsg_main -c 3 -t RASPI4.bin -a 0x7a000000
</li>
<li>5、运行后，根据提示使用对应的pty设备和UniProton进行通信：

	raspberrypi4-64 ~ # insmod mcs_km.ko
	raspberrypi4-64 ~ # ./rpmsg_main -c 3 -t RASPI4.bin -a 0x7a000000
	cpu:3, ld:7a000000, entry:7a000000, path:RASPI4.bin
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

如上所示，接着在另一个ssh窗口中，运行screen /dev/pts/2，此处在该窗口中，敲击回车即可触发demo程序。
</li>

可以在screen界面上看到输出结果如下：
```
hello, UniProton!
```
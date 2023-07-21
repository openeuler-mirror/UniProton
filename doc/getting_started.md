## 快速入门：开发Hello World
快速入门通过完成经典的“Hello World”把搭建UniProton开发环境的各步骤进行实例说明，让用户更直观地初步了解如何基于UniProton进行开发。

## 开发环境说明
开发平台：cortex_m4

芯片型号：stm32f407

OS版本信息：UniProton 22.03

集成开发环境：UniProton-docker

### 实现过程
以helloworld demo为例：[demo目录结构](../demos/helloworld/readme.txt)
<ol>
<li>进入到用户的工作目录，这里假设为/workspace/UniProton。</li>
<li>将UniProton的源码放在/workspace/UniProton目录下。</li>
<li>参考[编译指导](./UniProton_build.md) 准备编译环境以及libboundscheck库下载。</li>
<li>编译生成的libCortexM4.a文件在UniProton/output/UniProton/lib/cortex_m4/目录下，生成的libCortexMXsec_c.lib在UniProton/output/libboundscheck/lib/cortex_m4/目录下。将这两个静态库文件拷贝到demos/helloworld/libs目录下。</li>
<li>将UniProton/src/include目录下的头文件拷贝到demos/helloworld/include目录下。</li>
<li>将UniProton/src/config目录和UniProton/build/uniproton_config/config_m4目录下的文件拷贝到demos/helloworld/config目录下，并修改prt_config.c以及prt_config.h以适配用户功能，prt_config.h可配置os功能开关，按需裁剪。</li>
<li>demos/helloworld/bsp目录下可以新增板级驱动代码，demos/helloworld/build目录下配置编译构建相关内容，examples.ld为链接文件，根据单板内存地址等修改适配。</li>
<li>代码修改完成后，适配cmake，最后在build目录下运行sh build.sh即可在同级目录下生成helloworld可执行二进制文件。</li>
<li>加载到单板上运行可执行文件helloworld。</li>
</ol>

### Hello World 示例程序
在main.c文件中编写Hello World示例程序，如参考内容如下（如前所述，PRT_HardBootInit/PRT_HardDrvInit接口中添加用户自定义硬件模块初始化程序，PRT_AppInit接口中添加用户业务代码）：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "rtt_viewer.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "test.h"

void helloworld_task(U32 uwParam1, U32 uParam2, U32 uwParam3, U32 uwParam4)
{
    printf("hello world!\n");
    while (1) {
        PRT_TaskDelay(10);
    }

    return;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskPid;
    struct TskInitParam stInitParam = {helloworld_task, 10, 0, {0}, 0x500, "TaskA", 0};

    ret = PRT_TaskCreate(&taskPid, &stInitParam);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(taskPid);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    RttViewerInit();
    RttViewerModeSet(0, RTT_VIEWER_MODE_BLOCK_IF_FIFO_FULL);

    return OS_OK;
}

U32 g_testRandStackProtect;
void OsRandomSeedInit(void)
{
#if defined(OS_OPTION_RND)
    U32 ret;
    U32 seed;
    seed = PRT_ClkGetCycleCount64();
    g_testRandStackProtect = rand_r(&seed);
    ret = PRT_SysSetRndNum(OS_SYS_RND_STACK_PROTECT,g_testRandStackProtect);
#endif
}

void OsGlobalDataInit(void)
{
}

void PRT_HardBootInit(void)
{
    OsGlobalDataInit();
    OsRandomSeedInit();
}

U32 PRT_Printf(const char *format, ...)
{
    va_list vaList;
    char buff[0x200] = { 0 };
    S32 count;
    U32 ret;

    va_start(vaList, format);
    count = vsprintf_s(buff, 0x200, format, vaList);
    va_end(vaList);

    if (count == -1) {
        return OS_ERROR;
    }

    RttViewerWrite(0, buff, count);

    return count;
}

S32 main(void)
{
    return OsConfigStart();
}

```

### 结果验证
在docker环境中使用demos/helloworld/build/build_app.sh脚本编译整个工程后生成的可执行文件helloworld加载载到单板上运行，单板连接arm仿真器，打开jlink软件，可以在软件界面上看到输出结果如下：
```
hello world!
```
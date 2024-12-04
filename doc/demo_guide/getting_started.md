# 快速入门：开发 Hello World
快速入门通过完成经典的“Hello World”把搭建 UniProton 开发环境的各步骤进行实例说明，让用户更直观地初步了解如何基于 UniProton 进行开发。

## 开发环境说明
- 开发平台：cortex_m4
- 芯片型号：stm32f407
- OS版本信息：UniProton 24.03-LTS
- 集成开发环境：UniProton-docker

## 实现过程
以 cortex_m4 helloworld demo 为例：[demo目录结构](../demos/m4/readme.txt)

1. 进入到用户的工作目录，这里假设为 /workspace/UniProton。
2. 将 UniProton 的源码放在 /workspace/UniProton 目录下。
3. 参考[编译指导](./UniProton_build.md) 准备编译环境准备，推荐直接使用docker镜像编译。
4. 修改 demos/m4/config 目录下prt_config.c 以及 prt_config.h 以适配用户功能，prt_config.h 可配置 os 功能开关，按需裁剪。
5. demos/m4/bsp 目录下可以新增板级驱动代码，demos/m4/build 目录下配置编译构建相关内容，examples.ld 为链接文件，根据单板内存地址等修改适配。
6. demos/m4/build 目录下 build_app.sh用于编译构建demo生成对应二进制镜像文件，build_static.sh用于自动下载、拷贝依赖文件并编译OS内核，build_app.sh会自动调用build_static.sh。
7. 代码修改完成后，适配 cmake，最后在 build 目录下运行 `sh build_app.sh` 即可在同级目录下生成 helloworld 可执行二进制文件。
8. 加载到单板上运行可执行文件 helloworld。

## Hello World 示例程序
在 main.c 文件中编写 Hello World 示例程序，如参考内容如下（如前所述，PRT_HardBootInit/PRT_HardDrvInit 接口中添加用户自定义硬件模块初始化程序，PRT_AppInit 接口中添加用户业务代码）：
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

## 结果验证
在 docker 环境中使用 demos/helloworld/build/build_app.sh 脚本编译整个工程后生成的可执行文件 helloworld 加载载到单板上运行，单板连接 arm 仿真器，打开 jlink 软件，可以在软件界面上看到输出结果如下：
```bash
hello world!
```
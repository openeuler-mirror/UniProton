## 快速入门：开发Hello World
快速入门通过完成经典的“Hello World”把搭建UniProton开发环境的各步骤进行实例说明，让用户更直观地初步了解如何基于UniProton进行开发。

## 开发环境说明
开发平台：cortex_m4

OS版本信息：Linux
集成开发环境：UniProton-docker
### 实现过程
<ol>
<li>进入到用户的工作目录，这里假设为/workspace/UniProton。</li>
<li>将UniProton的源码放在/workspace/UniProton目录下。</li>
<li>在docker环境中搭建工程。</li>
<li>配置链接器配置文件（.lds文件）。</li>
<li>配置功能模块，用户需要按照自己实际使用模块配置功能模块。</li>
<li>在该工程下规划各开发模块的目录结构，如在/UniProton/src/config/prt_config.c文件中实现用户程序。</li>
<li>在docker中编译整个工程，编译完成后在/UniProton/output目录下生成可执行文件hello。</li>
<li>加载到单板上运行可执行文件hello。</li>
</ol>

### Hello World 示例程序
在main.c文件中编写Hello World示例程序，如参考内容如下（如前所述，PRT_HardBootInit/PRT_HardDrvInit接口中添加用户自定义硬件模块初始化程序，PRT_AppInit接口中添加用户业务代码）：
```c
#include "prt_task.h"
#include "prt_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

U32 PRT_HardDrvInit(void)
{
    return OS_OK;
}
void PRT_HardBootInit()
{

}
void OsTskUser(void)
{
    printf("\n\r Hello World !\n\r");
}
 
U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskId; 
    struct TskInitParam taskParam;

    taskParam.taskEntry = (TskEntryFunc)OsTskUser;
    taskParam.stackSize = 0x800;
    taskParam.name = "uniUserTask";
    taskParam.taskPrio = OS_TSK_PRIORITY_05;
    taskParam.stackAddr = 0;

    ret = PRT_TaskCreate(&taskId, &taskParam);
    if (ret != OS_OK) {
        return ret;
    }
 
    return OS_OK;
}
 
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
```

### 结果验证
在docker中编译整个工程后生成的可执行文件hello下载到单板上运行，输出结果如下：
```
Hello world!
```
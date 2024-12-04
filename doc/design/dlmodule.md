# 动态加载使用方法
## 1 约束
1）、当前仅支持ELF格式的共享库文件（.so）的动态加载方案。
2）、当前仅支持共享库文件的单向依赖，且需要在共享库源码中使用dlopen去调用被依赖共享库。
3）、当前仅支持ARMv8和x86_64架构。
4）、依赖代理文件系统，所以需要在代理文件系统启动后才可以使用。
5）、当前加载模式不支持延时绑定，默认只能立刻绑定。
6）、当前不支持gdb

## 2 编译并使用
1）、在对应的款型配置文件defconfig里面放开两个编译宏：
CONFIG_OS_ARCH_CPU64
CONFIG_OS_OPTION_DYNAMIC_MODULE

2）、共享库编译需要使用-fPIC和-nostdlib两个编译选项

3）、在自己的任务里面直接使用dlopen打开对应的共享库文件（文件路径是Linux下的路径），使用dlsym找到需要的使用的符号，最后使用dlclose关闭本次加载，如果有失败使用dlerror获取失败原因，注意失败原因只能记录最近的一次，后续错误会覆盖，所以需要及时获取。

## 3 举例：

```
#include <dlfcn.h>

typedef int (*GetTest)(void);

int main(void)
{
    void *dyn = dlopen("/opt/libtest.so", 0)；
    if (dyn == NULL) {
        printf("dlopen fail(%s)\n", dlerror());
        return -1;
    }
    GetTest test = dlsym(dyn, "GetTest");
    if (test == NULL) {
        printf("dlsym fail(%s)\n", dlerror());
        return -1;
    }
    int a = test();
    dlclose(dyn);
    return 0;
}

```


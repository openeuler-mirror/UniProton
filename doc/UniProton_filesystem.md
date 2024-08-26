# Uniproton Filesystem使用指南

## 1 整体方案：
当前UniProton filesystem功能支持三套方案，一套是代理文件系统，一套本地vfs，另一套是文件系统共存

代理文件系统原理和使用可以参考[基于UniProton的RPC服务](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/services/rpc.html)

本地vfs基于nuttx移植，目前底层仅支持对fat文件系统格式进行读写

文件系统共存指的是可以同时使用本地文件系统和代理文件系统


## 2 编译时使能文件系统功能：
单独用代理文件系统，需要在defconfig 设置
```
CONFIG_OS_OPTION_PROXY=y
```

单独使用文件系统，需要开启的宏开关比较多，不在此一一列举，可以参考ascend310b的defconfig设置，同时新增如下设置：
```
CONFIG_OS_OPTION_SEM_RECUR_PV=y
CONFIG_OS_OPTION_NUTTX_VFS=y
CONFIG_OS_OPTION_DRIVER=y
```

使用网络共存，同时在defconfig中开启上述选项

## 3 文件系统使用：
单独使用代理文件系统，开启开始对应宏开关即可使用，无需任何初始化

单独使用本地vfs文件系统，除开启对应宏开关外，还需要进行文件系统初始化和mount操作，可参考[m4 fatfs使用](https://gitee.com/openeuler/UniProton/tree/master/demos/m4/apps/fatfs)

文件系统共存使用，需要同时在defconfig中开启代理文件系统和本地vfs文件系统选项。目前是通过路径来区分走本地vfs还是走代理文件系统。如果想自定义路径走本地vfs还是代理文件系统，可修改src/fs/vfs/fs_proxy.c的proxyPath接口实现。目前/tmp/路径下走的是代理文件系统，其他路径走本地vfs文件系统。
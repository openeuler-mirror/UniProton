# Uniproton Network使用指南

## 1 整体方案：
当前UniProton network功能支持两套方案，一套是代理网络，另一套是网络共存

代理网络原理和使用可以参考[基于UniProton的RPC服务](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/services/rpc.html)

网络共存指的是同时使用本地网络和代理网络，其中本地网络适配的是轻量级网络协议栈lwip


## 2 编译时使能网络功能：
单独用代理网络，需要在defconfig 设置
```
CONFIG_OS_OPTION_PROXY=y
```

使用网络共存，需要在defconfig 设置
```
CONFIG_OS_OPTION_PROXY=y
CONFIG_OS_SUPPORT_NET=y
```

## 3 网络共存使用适配：
使用代理网络，开启对应defconfig 即可直接使用

使用网络共存方案，除defconfig开启上述宏开关外，本地网络在使用上存在适配点：

(1)网卡驱动适配：
UniProton目前仅支持i210和i40e两款网卡驱动，比较有限。用户如果需要走其他网卡，需要参考UniProton网络驱动接口，新增对应网卡驱动

(2)网卡驱动注册：
UniProton在网络适配层提供了ethernetif_api_register接口，用户需将新增实现的网络驱动接口注册

(3)使用参考：
网络共存测试用例可参考testsuites/lwipTest目录，在完成网卡驱动注册和对lwip进行必要初始化后，即可使用共存方案

在共存方案中，本地网络跟代理网络在使用上基本没有区别，唯一的区别点是本地网络在创建socket之后，需要调用setsockopt接口，将socket绑定到指定网卡上，走本地协议栈。未调用setsockopt接口的socket会话默认以代理的形式创建，走linux协议栈

```
const char *device_name = "ethxxxyyy";
int ret = setsockopt(g_socketfd, SOL_SOCKET, SO_BINDTODEVICE, device_name, strlen(device_name) + 1);
```
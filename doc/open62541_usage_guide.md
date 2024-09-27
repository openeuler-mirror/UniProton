# open62541使用指南

## 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton master
- 集成开发环境：UniProton-docker

## open62541使用
使能配置项CONFIG_OS_SUPPORT_OPC_UA，在UniProton/demos/hi3093/build目录下，执行命令：
```bash
sh build_app.sh opcuaTest
```
open62541源码下载存放在UniProton/demos/hi3093/component目录下，测试代码在UniProton/testsuites/opcuaTest/目录下。其中opcua_server.c是在UniProton服务端运行的测试代码，在build下编译生成测试程序opcuaTest.elf。opcua_client.c是在客户端运行的测试代码，可将其移植到linux系统上依赖open62541源码进行编译，编译完成后运行即可实现与UniProton服务端的OPC UA通信。
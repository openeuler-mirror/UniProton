# libxml2库使用指南

## 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton master
- 集成开发环境：UniProton-docker

## libxml2使用

使能CONFIG_OS_SUPPORT_LIBXML2，在UniProton/demos/hi3093/build目录下，执行命令：
```bash
sh build_app.sh UniPorton_test_libxml2_interface
```
libxml2下载存放在UniProton/demos/hi3093/component目录下，在build下生成libxml2测试程序UniPorton_test_libxml2_interface.bin。
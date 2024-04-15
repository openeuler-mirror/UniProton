# ccl库使用指南

## 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton master
- 集成开发环境：UniProton-docker

## libccl使用

在UniProton/demos/hi3093/build目录下，执行命令：
```bash
sh build_app.sh libcclTest
```
libccl下载存放在UniProton/src/component目录下，在build目录下生成测试程序libcclTest.bin，在运行测试程序时，需将ccl/demo目录下的simple.conf和example.conf上传到测试环境，且与测试程序在同一目录。
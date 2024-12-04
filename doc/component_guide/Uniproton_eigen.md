# Eigen库使用指南

## 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton master
- 集成开发环境：UniProton-docker

## 1 使用前准备
在使用Eigen之前，需要先按照libc++编译指南完成libc++编译，因为Eigen是依赖C++的。

## 2 Eigen测试程序编译
eigen-3.4.0源码存放在UniProton/demos/hi3093/component目录下，其中eigen-3.4.0/doc/examples/目录下的测试用例拷贝到了UniProton/testsuites/eigen-test目录下，用例cpp文件中的main函数在编译时需要修改为test_前缀命名函数，并在eigen_test.cc的Init中调用。比如classBlock.cpp中的main函数可修改名称为test_class_block。

使能CONFIG_OS_SUPPORT_EIGEN，在UniProton/demos/hi3093/build目录下，执行编译命令,编译Eigen测试程序eigenTest.elf：
```bash
sh build_app.sh eigenTest
```
# libc++编译与使用指南

## 开发环境说明
- 开发平台：armv8
- 芯片型号：hi3093
- OS版本信息：UniProton master
- 集成开发环境：UniProton-docker

## 1 编译前准备

### 1.1 yaml安装
编译环境需提前安装yaml，如果编译环境中未安装pip，需先安装pip后再安装yaml：
```bash
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py
pip3 install pyyaml
```

### 1.2 环境变量设置
libc++编译环境变量的设置在UniProton/demos/hi3093/build/libcxx_build.sh中，该脚本实现了libc++的编译过程。首先下载开源gcc源码，并根据libstdc++-uniproton.patch对源码进行修改。随后设置编译相关环境变量并进行编译。其中几个关键变量设置如下：
```bash
GCC_PATH：gcc源码下载目录，指定该变量后gcc源码将下载到该目录下
UNIPROTON_PATH：UniProton源码存放目录，可根据实际源码存放位置进行设置
CXX_COMPILE_PATH：aarch64-openeuler-linux工具链安装目录，默认安装目录为/opt/openeuler/oecore-x86_64/sysroots/x86_64-openeulersdk-linux/usr
```
以上环境变量可根据实际情况进行设置，也可直接使用脚本中的默认设置进行编译。

## 2 libc++及测试程序编译

### 2.1 测试用例设置
C++相关测试用例位于UniProton/testsuites/cxx-test目录下，通过修改tools/config.yaml，可修改测试用例范围：
```bash
- ../misc
# - ../20_util
# - ../23_containers/stack
# - ../23_containers/queue
# - ../23_containers/priority_queue
# - ../23_containers/vector/modifiers
```
如上，通过去掉文件中对应行数的注释，可实现misc目录下测试用例的编译。如果想编译其他目录下的用例，可做相应修改。

### 2.2 测试程序编译
使能CONFIG_OS_SUPPORT_CXX，在UniProton/demos/hi3093/build目录下，执行命令：
```bash
sh build_app.sh cxxTest
```
编译生成的libc++位于UniProton/demos/hi3093/component/libcxx目录下，测试程序为build目录下的cxxTest.elf。
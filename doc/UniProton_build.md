# 一、Linux下的编译
## 1.1 搭建Linux编译环境
##### 【注】当前所有编译器的安装目录都在/opt/buidltools下，首先需要创建该目录并设置权限:mkdir -p /opt/buildtools && chmod -R 755 /opt/buildtools
## 软件要求
1. SUSE Linux Enterprise Server
2. python 3.4+
3. Cmake
4. GNU Arm Embedded Toolchain编译器，用于代码编译。
## 1.2 安装编译器&构建器
- #### 安装GNU Arm Embedded Toolchain编译器。
1. 下载编译器.
- 当前开源版本只涉及32位，官方下载地址为：[GNU Arm Embedded Toolchain编译器](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)，指定版本:10-2020-q4-major。
2. 解压编译器.
- 可以参考如下命令完成解压：
<br>
		tar -xvf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 -C /opt/buildtools

## 1.3 安装Cmake
1. 通过官网进行下载[Cmake](https://cmake.org/download/)，下载指定版本3.20.5
2. 以tar包模式举例说明:
- 通过命令进行解压：
- tar -xvf cmake-3.20.5-Linux-x86_64.tar.gz -C /opt/buildtools

## 1.4 安装Python
##### 下面以python3.8为例介绍安装方法。
1. 通过[官网下载python源码包](https://gitee.com/link?target=https%3A%2F%2Fwww.python.org%2Fftp%2Fpython%2F3.8.5%2FPython-3.8.5.tgz)。

2. 解压源码包。
  - 参考如下命令完成解压，将压缩包名替换为实际下载的源码包名：
```
    tar -xf Python-3.8.5.tgz
```
3. 检查依赖。
  - 解压后进入到目录中，执行./configure命令以检查编译与安装python所需的依赖：
```
    cd Python-3.8.5
    ./configure
```
- 如果没有报错就继续下一步操作，如果存在报错就根据提示安装依赖。
4. 编译&安装python。
```
	sudo make
	sudo make install
```
5. 检查python版本并正确链接python命令。
```
	python --version
```
- 如果显示的不是刚刚安装的python版本，则需要执行以下命令来正确链接python命令。

- a. 获取python目录，例如对于python 3.8.5，执行如下命令。

```
	which python3.8
```
- b. 链接python命令到刚刚安装的python包。

	将以下命令中的 "python3.8-path" 替换为 "which python3.8" 命令执行后的回显路径：

```
	cd /usr/bin && sudo rm python && sudo ln -s "python3.8-path" python
```
- c. 再次检查python版本。

```
	python --version
```
## 1.5 安装pip包管理工具。

##### 如果pip命令不存在，可以下载pip源码包进行安装。pip依赖setuptools，如果setuptools不存在，也需要安装。

1. 源码包方式安装：

- 安装setuptools。

- 点击[setuptools源代码包下载地址](https://gitee.com/link?target=https%3A%2F%2Fpypi.org%2Fproject%2Fsetuptools%2F)，可以参考下面的命令进行安装：

```
	sudo unzip setuptools-50.3.2.zip
	cd setuptools
	sudo python setup.py install
```

2. 安装pip。

- 点击[pip源代码包下载地址](https://gitee.com/link?target=https%3A%2F%2Fpypi.org%2Fproject%2Fpip%2F)，可以参考下面的命令进行安装：
```
        sudo tar -xf pip-20.2.4.tar.gz
        cd pip-20.2.4
        sudo python setup.py install
```

## 1.6 Linux下编译流程
##### 参照上述步骤完成环境搭建后，即可按以下步骤完成编译。
1. 下载UniProton代码
- git clone https://gitee.com/openeuler/UniProton.git
2. 下载libboundscheck, 按照[指导](../platform/README.md)操作
3. 执行编译
- 进入到UniProton根目录下执行命令即可
```
        python build.py m4
```


# 二、提供镜像，用户自行下载
1. 在虚拟机操作命令：

	`docker pull swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/uniproton:v001`
	- 执行完成之后，创建容器并进入(默认挂载当前执行命令的目录为容器内的/home/uniproton目录)

	`docker run -it -v $(pwd):/home/uniproton swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/uniproton:v001`
2. 下载代码
    - 下载UniProton代码
        `git clone https://gitee.com/openeuler/UniProton.git`
    - 下载libboundscheck, 按照[指导](../platform/README.md)操作

3. 执行编译
	- 进入到UniProton根目录下执行命令即可
```
        python build.py m4
```

# 三、 编译结果
	生成的静态库文件存放在output/UniProton/lib/cortex_m4目录下。

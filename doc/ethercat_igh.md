# UniProton ethercat使用指南

目前只有x86_64支持ethercat功能

## 1 编译时使能ethercat功能：

defconfig 设置CONFIG_OS_SUPPORT_ETHERCAT=y

## 2 igh命令行使用：

### 2.1 配置编译环境

参考[基于SDK的应用开发](https://openeuler.gitee.io/yocto-meta-openeuler/master/getting_started/index.html#sdk)配置x86_64编译环境。

### 2.2 下载igh ethercat代码

git clone https://gitlab.com/Tim-Tianyu/ethercat.git

### 2.3 编译ethercat可执行

运行./build.sh编译，可执行文件ethercat生成在build文件夹内。

### 2.4 运行

正常使用命令行宏依赖mcs混合部署, 参考[mcs](https://gitee.com/openeuler/mcs)。

可执行文件ethercat复制到运行环境的/usr/local/bin文件夹中, chmod +x添加执行权限, 使用mica混合部署框架拉起Uniproton后，命令行即可正常使用。
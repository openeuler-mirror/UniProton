## 前置条件

原生 ubuntu 22.04.1

 

## oebuild安装

参考 "运行环境准备"，安装python3 pip oebuild

https://openeuler.gitee.io/yocto-meta-openeuler/master/yocto/oebuild.html#id2

参考 "如何编译一个标准镜像"，尝试构建默认镜像（操作时需要切换到普通用户权限）

https://openeuler.gitee.io/yocto-meta-openeuler/master/yocto/oebuild.html#id4

例子：
```sh
mkdir example
cd ./example
oebuild init tmp
cd ./tmp 
oebuild update
oebuild generate
cd ./build/aarch64-std
oebuild bitbake openeuler-image
```

结果显示没有error messages，代表构建默认镜像成功，编辑环境正常



## 编译 uniproton

在 build/aarch64-std 文件夹内，使用 oebuild bitbake, 进入编辑环境

下载并编译 uniproton 个人分支代码:

```sh
git clone https://gitee.com/MegamindLS/UniProton.git
cd UniProton
git checkout -b tmp origin/tmp
cd demos/uvpck/build
sh ./build_app.sh
```

镜像路径: build/aarch64-std/UniProton/demos/uvpck/build/uvpck.bin
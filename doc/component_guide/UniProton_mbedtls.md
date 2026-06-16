# UniProton Mbed TLS使用指南

## 1 功能描述

Mbed TLS是轻量级TLS/DTLS安全通信组件。当前UniProton在sd3403上接入Mbed TLS v2.16.8，提供DTLS 1.2 PSK服务端基础能力，用于验证UniProton任务、定时、熵源和代理socket路径上的DTLS握手与应用数据收发。

当前验证用例为单次DTLS PSK server：

- UniProton侧监听UDP端口5658。
- Host侧通过OpenSSL DTLS PSK client连接。
- 双方完成DTLS 1.2握手。
- Host发送`Hi Server`。
- UniProton回复`Hi Client`。

该能力当前主要用于sd3403代理网络路径验证，不包含证书认证、TCP TLS client/server、完整mbedtls示例集等通用能力。

## 2 目录结构

Mbed TLS适配层位于：

```text
src/component/mbedtls/
├── CMakeLists.txt
└── port/
    ├── entropy_hardware_poll.c
    ├── net_sockets_uniproton.c
    ├── timing_alt.c
    ├── timing_alt.h
    └── uniproton_mbedtls_config.h
```

DTLS验证用例位于：

```text
testsuites/mbedtls-test/
├── CMakeLists.txt
├── host_dtls_client.py
├── mbedtls_dtls_server_test.c
└── mbedtls_dtls_server_test.h
```

第三方Mbed TLS源码不随仓库提交。编译时由`src/component/mbedtls/CMakeLists.txt`通过`FetchContent`下载到：

```text
src/component/mbedtls/mbedtls-2.16.8
```

该目录已加入`.gitignore`。

## 3 宏控开关

Mbed TLS组件通过defconfig宏控制。以sd3403为例，配置文件为：

```text
build/uniproton_config/config_armv8_sd3403/defconfig
```

开启Mbed TLS组件：

```text
CONFIG_OS_SUPPORT_MBEDTLS=y
```

该宏定义在：

```text
src/security/Kconfig
```

sd3403 demo构建`APP=mbedtls`时会检查该宏。如果未开启，会在CMake配置阶段报错：

```text
PLEASE ENABLE CONFIG_OS_SUPPORT_MBEDTLS
```

## 4 依赖说明

当前sd3403 DTLS PSK server验证依赖如下能力：

- `CONFIG_OS_OPTION_OPENAMP=y`：提供sd3403与Linux侧通信基础。
- `CONFIG_OS_OPTION_PROXY=y`：提供代理socket能力，DTLS server通过该路径收发UDP报文。
- `CONFIG_OS_OPTION_POSIX=y`：提供socket相关POSIX接口声明和基础适配。
- `CONFIG_OS_OPTION_TASK=y`和`CONFIG_OS_OPTION_TICK=y`：提供任务调度和DTLS重传定时能力。

组件内部适配内容：

- `net_sockets_uniproton.c`：实现Mbed TLS net_sockets接口，底层使用UniProton代理socket。
- `timing_alt.c`：实现Mbed TLS timing接口，底层使用UniProton tick/cycle。
- `entropy_hardware_poll.c`：提供Mbed TLS硬件熵源回调。
- `uniproton_mbedtls_config.h`：裁剪Mbed TLS配置，仅开启DTLS 1.2 PSK server验证所需模块。

Host侧验证依赖OpenSSL命令行工具：

```bash
openssl version
```

## 5 配置文件作用

UniProton侧Mbed TLS配置文件为：

```text
src/component/mbedtls/port/uniproton_mbedtls_config.h
```

Mbed TLS 2.x本身通过`MBEDTLS_CONFIG_FILE`宏选择配置头文件。Mbed TLS源码中大量`.c`文件和公共头文件都有如下逻辑：

```c
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
```

如果不定义`MBEDTLS_CONFIG_FILE`，Mbed TLS会使用上游默认配置`mbedtls/config.h`。该默认配置会打开较多通用功能，和UniProton当前只编译DTLS PSK server所需源码文件的裁剪方式不匹配，可能导致编译、链接或运行时配置不一致。

当前在`src/component/mbedtls/CMakeLists.txt`中通过统一的`mbedtls_config`接口target定义该宏：

```cmake
target_compile_definitions(mbedtls_config INTERFACE
    MBEDTLS_CONFIG_FILE="uniproton_mbedtls_config.h"
)
```

`mbedtls_component`和`mbedtlsTest`都链接`mbedtls_config`，因此只有Mbed TLS组件和Mbed TLS测试代码会使用该配置。`rpmsg`、`bsp`、`config`、`proxy`、`mica`等其他sd3403目标不会继承该宏。

该配置文件产生的主要效果如下：

- 将Mbed TLS配置切换为UniProton自定义裁剪配置，而不是上游默认`mbedtls/config.h`。
- 打开DTLS 1.2和PSK密钥交换能力：`MBEDTLS_SSL_PROTO_DTLS`、`MBEDTLS_SSL_PROTO_TLS1_2`、`MBEDTLS_KEY_EXCHANGE_PSK_ENABLED`。
- 打开DTLS PSK server所需TLS模块：`MBEDTLS_SSL_SRV_C`、`MBEDTLS_SSL_TLS_C`。
- 固定当前验证使用的密码套件：`MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256`。
- 打开当前密码套件依赖的算法模块：AES、CBC cipher、SHA256、MD、CTR_DRBG、entropy等。
- 启用UniProton侧替代实现：`MBEDTLS_NET_C`、`MBEDTLS_TIMING_ALT`、`MBEDTLS_ENTROPY_HARDWARE_ALT`。
- 关闭默认平台熵源：`MBEDTLS_NO_PLATFORM_ENTROPY`，避免依赖标准OS的熵源接口。
- 限制TLS记录内容长度：`MBEDTLS_SSL_MAX_CONTENT_LEN 1024`，降低测试镜像内存占用。
- 打开错误字符串模块：`MBEDTLS_ERROR_C`，用于测试日志中打印可读错误信息。

因此，`uniproton_mbedtls_config.h`不是普通业务头文件，而是Mbed TLS编译期功能裁剪和平台适配开关的统一入口。所有被编译进`mbedtls_component`的上游Mbed TLS源码文件，以及包含Mbed TLS公共头文件的`mbedtlsTest`，都需要看到同一份配置，避免出现源码编译配置、头文件声明配置和链接对象不一致的问题。

## 6 编译方法

在Docker容器中构建sd3403 mbedtls app：

```bash
cd /home/uniproton/UniProton/demos/sd3403/build
sh build_app.sh mbedtls
```

在Host侧可通过如下命令触发容器构建：

```bash
sg docker -c "docker exec recursing_jennings bash -c 'cd /home/uniproton/UniProton/demos/sd3403/build && sh build_app.sh mbedtls'"
```

构建产物：

```text
demos/sd3403/build/mbedtls.elf
demos/sd3403/build/mbedtls.bin
demos/sd3403/build/mbedtls.asm
```

首次构建会下载Mbed TLS v2.16.8，CMake日志中可看到：

```text
--- Download mbedtls v2.16.8
--- End download mbedtls
```

## 7 上板验证

sd3403上板验证需要将`mbedtls.elf`拷贝到板端`/etc/mica/`，并通过mica启动CPU3上的UniProton实例。启动后向`/dev/ttyRPMSG0`写入任意字符触发测试入口。

Host侧可使用测试脚本连接UniProton DTLS server：

```bash
python3 testsuites/mbedtls-test/host_dtls_client.py
```

也可直接使用OpenSSL命令：

```bash
openssl s_client \
    -dtls1_2 \
    -connect 192.168.7.2:5658 \
    -psk_identity testserver1 \
    -psk 3131323233333434353536363737383831313232333334343535363637373838 \
    -quiet
```

其中：

- PSK identity为`testserver1`。
- PSK ASCII字符串为`11223344556677881122334455667788`。
- OpenSSL `-psk`参数需要传入PSK ASCII字符串的十六进制形式。

验证成功时，Host侧输出：

```text
Hi Client
```

UniProton串口日志输出：

```text
[MBEDTLS_TEST] DTLS PSK server start, port=5658
[MBEDTLS_TEST] waiting for DTLS client
[MBEDTLS_TEST] handshake start
[MBEDTLS_TEST] handshake ok
[MBEDTLS_TEST] read 10 bytes: Hi Server
[MBEDTLS_TEST] write 9 bytes: Hi Client
[MBEDTLS_TEST] DTLS PSK server test passed
```

## 8 当前验证结果

当前已在sd3403板端完成验证：

- 构建命令：`sh build_app.sh mbedtls`
- 运行镜像：`demos/sd3403/build/mbedtls.elf`
- Host client返回码：`0`
- Host收到：`Hi Client`
- 板端日志显示：`DTLS PSK server test passed`

## 9 限制说明

当前接入是面向sd3403 DTLS PSK server验证的最小适配，存在如下限制：

- 仅编译DTLS 1.2 PSK server验证所需Mbed TLS源码文件。
- 当前未开启X.509、RSA/ECC证书链、TLS TCP、client模式等能力。
- 当前使用代理socket路径，依赖sd3403 Linux侧网络转发能力。
- 验证用例为单次server，完成一次握手和收发后退出。
- `entropy_hardware_poll.c`当前提供基础熵源适配，用于测试验证；产品化场景应接入硬件TRNG或平台安全随机源。

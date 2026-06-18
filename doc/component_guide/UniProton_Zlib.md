# UniProton Zlib组件支持设计

## 1 背景

Zlib是通用的数据压缩库，提供压缩、解压、校验和以及gzip文件操作接口。为支持UniProton在sd3403场景下的数据压缩能力，需要在UniProton中新增Zlib组件，并完成基础接口和端到端功能验证。

本设计基于UniProton现有组件接入方式实现。Zlib作为UniProton组件编译进内核静态库，测试用例作为sd3403 demo应用的一部分单独链接，避免组件源码在内核库和demo应用中重复编译。

## 2 设计目标

Zlib组件支持目标如下：

- 在UniProton中提供Zlib 1.2.11公共接口。
- Zlib源码放置在`src/component/zlib`，符合UniProton组件目录布局。
- 通过`CONFIG_OS_SUPPORT_ZLIB`控制组件是否编译。
- zlib源码只通过`src/CMakeLists.txt`进入UniProton内核静态库，demo侧不重复添加组件源码。
- 为sd3403提供`zlib`测试应用，覆盖基础接口测试和端到端压缩/解压功能测试。
- 通过sd3403上板运行验证功能正确性。

## 3 非目标

当前设计不包含以下内容：

- 不修改zlib算法源码实现。
- 不为zlib新增UniProton私有API封装。
- 不引入新的堆管理实现，zlib动态内存使用UniProton libc中的`malloc`和`free`。
- 不在sd3403测试中依赖实际文件系统路径进行gzip文件读写验证。

说明：zlib的`gzopen`、`gzread`、`gzwrite`、`gzclose`等源码已纳入组件编译，符号可链接。sd3403当前功能验证重点是内存压缩/解压路径，避免测试对板端文件系统挂载和路径权限产生额外依赖。

## 4 目录结构

zlib组件目录：

```text
src/component/zlib/
├── CMakeLists.txt
├── include/
│   ├── zlib.h
│   ├── zconf.h
│   ├── zutil.h
│   ├── deflate.h
│   ├── inflate.h
│   └── ...
└── src/
    ├── adler32.c
    ├── compress.c
    ├── crc32.c
    ├── deflate.c
    ├── gzclose.c
    ├── gzlib.c
    ├── gzread.c
    ├── gzwrite.c
    ├── infback.c
    ├── inffast.c
    ├── inflate.c
    ├── inftrees.c
    ├── trees.c
    ├── uncompr.c
    └── zutil.c
```

sd3403测试目录：

```text
testsuites/zlib-test/
├── CMakeLists.txt
├── zlib_test.c
└── zlib_test.h
```

sd3403 demo入口复用已有OpenAMP应用：

```text
demos/sd3403/apps/openamp/main.c
```

## 5 架构设计

zlib组件接入分为两层：

- 组件层：`src/component/zlib`编译zlib源码，并进入UniProton内核静态库。
- 测试层：`testsuites/zlib-test`编译测试用例，sd3403 `APP=zlib`时链接到最终ELF。

构建关系如下：

```text
build_app.sh zlib
        |
        v
build_static.sh sd3403
        |
        v
python build.py sd3403
        |
        v
src/CMakeLists.txt
        |
        v
src/component/zlib/CMakeLists.txt
        |
        v
libSD3403.a
        |
        v
demos/sd3403/CMakeLists.txt + testsuites/zlib-test
        |
        v
demos/sd3403/build/zlib.elf
```

关键约束：

- `src/component/zlib`只能在`src/CMakeLists.txt`中添加。
- `demos/sd3403/CMakeLists.txt`不能再次`add_subdirectory(${HOME_PATH}/src/component/zlib ...)`。
- demo侧只添加`testsuites/zlib-test`测试对象，并通过头文件目录引用zlib接口声明。

这样可以保证zlib对象只存在于`libSD3403.a`中，避免同一源码同时出现在内核库和demo目标对象中。

## 6 编译接入

### 6.1 Kconfig配置

zlib配置项定义在：

```text
src/utility/Kconfig
```

配置项：

```text
config OS_SUPPORT_ZLIB
    bool "Whether support zlib module or not"
    default n
```

sd3403开启方式：

```text
build/uniproton_config/config_armv8_sd3403/defconfig
```

```text
CONFIG_OS_SUPPORT_ZLIB=y
```

### 6.2 UniProton源码树接入

`src/CMakeLists.txt`根据配置项添加zlib组件：

```cmake
if(${CONFIG_OS_SUPPORT_ZLIB})
    add_subdirectory(component/zlib)
endif()
```

`src/component/zlib/CMakeLists.txt`负责：

- 定义zlib源码列表。
- 创建`zlib_component`对象库。
- 将`zlib_component`追加到`ALL_OBJECT_LIBRARYS`，确保最终进入`libSD3403.a`。
- 设置zlib头文件路径。

关键逻辑：

```cmake
add_library(zlib_component OBJECT ${ZLIB_SRC})

list(APPEND ALL_OBJECT_LIBRARYS zlib_component)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
```

### 6.3 sd3403测试接入

`demos/sd3403/CMakeLists.txt`中新增`APP=zlib`分支。该分支只添加测试目录：

```cmake
elseif (${APP} STREQUAL "zlib")
    if (NOT "${CONFIG_OS_SUPPORT_ZLIB}" STREQUAL "y")
        message(FATAL_ERROR "PLEASE ENABLE CONFIG_OS_SUPPORT_ZLIB")
        return()
    endif()
    add_subdirectory(${HOME_PATH}/testsuites/zlib-test tmp)
    target_compile_options(rpmsg PUBLIC -DZLIB_TESTCASE)
    target_include_directories(rpmsg PUBLIC
        ${HOME_PATH}/testsuites/zlib-test
    )
    list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:zlibTest>)
```

测试目标`zlibTest`直接包含zlib头文件目录：

```cmake
target_include_directories(zlibTest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HOME_PATH}/src/component/zlib/include
)
```

## 7 内存与平台适配

Zlib默认通过`zcalloc`和`zcfree`分配内部状态和窗口内存。当前设计保持Zlib原始逻辑：

- `zcalloc`调用`malloc`。
- `zcfree`调用`free`。

UniProton libc中的`malloc`和`free`已经适配到UniProton内存管理：

- `malloc`底层调用`PRT_MemAlloc`。
- `free`底层调用`PRT_MemFree`。

因此zlib无需新增平台内存适配层。

## 8 接口范围

本次编译纳入的zlib源码覆盖以下接口类别：

- 校验接口：`adler32`、`crc32`。
- 一次性压缩/解压接口：`compress`、`compress2`、`compressBound`、`uncompress`、`uncompress2`。
- 流式压缩接口：`deflateInit`、`deflateInit2`、`deflate`、`deflateEnd`等。
- 流式解压接口：`inflateInit`、`inflateInit2`、`inflate`、`inflateEnd`等。
- gzip文件接口：`gzopen`、`gzread`、`gzwrite`、`gzclose`等。
- 工具接口：`zlibVersion`、`zlibCompileFlags`、`zError`。

## 9 测试设计

测试入口：

```text
testsuites/zlib-test/zlib_test.c
```

sd3403启动后，`demos/sd3403/apps/openamp/main.c`在`ZLIB_TESTCASE`打开时调用：

```c
zlib_test();
```

测试分为基础接口测试和端到端功能测试。

### 9.1 基础接口测试

基础接口测试覆盖：

- `zlibVersion`返回版本与`ZLIB_VERSION`一致。
- `zError`可以返回预期错误字符串。
- `compressBound`返回值不小于输入长度。
- `adler32`一次性计算与分段增量计算一致。
- `crc32`一次性计算与分段增量计算一致。

### 9.2 一次性压缩/解压端到端测试

测试流程：

```text
原始数据 -> compress2 -> 压缩数据 -> uncompress -> 还原数据 -> memcmp校验
```

验证点：

- `compress2`返回`Z_OK`。
- `uncompress`返回`Z_OK`。
- 还原长度等于原始数据长度。
- 还原内容与原始数据完全一致。

### 9.3 流式压缩/解压端到端测试

测试流程：

```text
原始数据 -> deflateInit/deflate/deflateEnd -> 压缩数据
压缩数据 -> inflateInit/inflate/inflateEnd -> 还原数据 -> memcmp校验
```

验证点：

- `deflateInit`返回`Z_OK`。
- `deflate`使用`Z_FINISH`后返回`Z_STREAM_END`。
- `deflateEnd`返回`Z_OK`。
- `inflateInit`返回`Z_OK`。
- `inflate`使用`Z_FINISH`后返回`Z_STREAM_END`。
- `inflateEnd`返回`Z_OK`。
- 输出长度和输出内容均与原始数据一致。

## 10 编译方法

在Docker容器内编译sd3403 zlib测试应用：

```bash
cd /home/uniproton/UniProton/demos/sd3403/build
sh build_app.sh zlib
```

Host侧触发容器编译示例：

```bash
sg docker -c "docker exec uniproton_cmsis_verify bash -c 'cd /home/uniproton/UniProton/demos/sd3403/build && sh build_app.sh zlib'"
```

构建产物：

```text
demos/sd3403/build/zlib.elf
demos/sd3403/build/zlib.bin
demos/sd3403/build/zlib.asm
```

## 11 上板验证方法

sd3403验证流程：

1. 将`zlib.elf`传到板端`/etc/mica/zlib.elf`。
2. 写入mica配置，`ClientPath=/etc/mica/zlib.elf`。
3. 确认CPU3 offline。
4. 清理旧mica实例。
5. 启动串口日志采集。
6. `mica create /etc/mica/sd3403.conf`。
7. `mica start sd3403`。
8. 向`/dev/ttyRPMSG0`写入任意字符触发UniProton测试。
9. 等待串口输出`[ZLIB_TEST] ALL TESTS PASSED`。
10. 停止并删除mica实例。

验证成功日志示例：

```text
[ZLIB_TEST] start
[ZLIB_TEST] version=1.2.11, compile_flags=0xa9
[ZLIB_TEST] [PASS] zlibVersion
[ZLIB_TEST] [PASS] zError
[ZLIB_TEST] [PASS] compressBound
[ZLIB_TEST] [PASS] adler32 incremental
[ZLIB_TEST] [PASS] crc32 incremental
[ZLIB_TEST] [PASS] compress2
[ZLIB_TEST] [PASS] uncompress
[ZLIB_TEST] [PASS] uncompress length
[ZLIB_TEST] [PASS] uncompress payload
[ZLIB_TEST] [PASS] deflateInit
[ZLIB_TEST] [PASS] deflate finish
[ZLIB_TEST] [PASS] deflateEnd
[ZLIB_TEST] [PASS] inflateInit
[ZLIB_TEST] [PASS] inflate finish
[ZLIB_TEST] [PASS] inflateEnd
[ZLIB_TEST] [PASS] inflate length
[ZLIB_TEST] [PASS] inflate payload
[ZLIB_TEST] ALL TESTS PASSED
```

## 12 风险与约束

### 12.1 构建重复风险

zlib源码必须只通过`src/CMakeLists.txt`进入内核库。如果同时在`demos/sd3403/CMakeLists.txt`中添加`src/component/zlib`，会造成组件源码重复编译，破坏UniProton组件构建边界。

正确做法：

- `src/CMakeLists.txt`负责组件源码。
- `demos/sd3403/CMakeLists.txt`负责测试对象。

### 12.2 配置一致性风险

`CONFIG_OS_SUPPORT_ZLIB`需要在内核静态库构建和demo CMake配置两个阶段保持一致。sd3403构建流程中，`build_app.sh`会先调用`build_static.sh`生成`libSD3403.a`，之后再配置demo目标。如果内核库未打开zlib而demo测试打开，则最终链接会出现zlib符号未定义。

### 12.3 gzip文件接口验证约束

`gz*`接口依赖`open`、`read`、`write`、`lseek`、`close`等文件接口。当前源码已纳入编译并可链接，但sd3403测试没有把实际文件系统读写作为通过条件。若后续需要验证`gzopen`到`gzread/gzwrite`的完整文件路径，需要先明确板端可用文件系统、挂载路径和权限。

## 13 后续扩展

后续可以补充以下能力：

- 增加`gz*`文件接口端到端测试。
- 将zlib测试扩展到更多架构或demo板卡。
- 增加更大数据量、多压缩等级、多flush模式测试。
- 增加异常输入测试，例如损坏压缩数据、输出缓冲区不足、非法参数等。

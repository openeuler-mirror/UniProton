# UniProton CMSIS-RTOS适配说明

CMSIS-RTOS适配层位于`src/component/cmsis`。`CONFIG_OS_SUPPORT_CMSIS=y`表示启用CMSIS组件，CMSIS-RTOS v1或v2由`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`选择。

| 版本 | 头文件配置 | 测试目录 | 设计文档 |
| --- | --- | --- | --- |
| CMSIS-RTOS v1 | `#define CMSIS_OS_VER 1` | `testsuites/cmsis/v1` | `doc/component_guide/UniProton_CMSIS_v1.md` |
| CMSIS-RTOS v2 | `#define CMSIS_OS_VER 2` | `testsuites/cmsis/v2` | `doc/component_guide/UniProton_CMSIS_v2.md` |

公共测试代码放在`testsuites/cmsis/common`，例如`cmsis_test_log.h`。版本差异较大的CMSIS API调用分别放在v1/v2目录内；确实可复用的日志和辅助逻辑保持公共化，不能复用的场景由`CMSIS_OS_VER`和目录选择隔离。

sd3403构建命令：

```bash
sg docker -c "docker exec uniproton_cmsis_verify bash -c 'cd /home/uniproton/UniProton/demos/sd3403/build && sh build_app.sh cmsis'"
```

组件编译开关仍使用目标平台`defconfig`中的：

```text
CONFIG_OS_SUPPORT_CMSIS=y
```

版本选择只使用`src/component/cmsis/cmsis_os.h`中的`CMSIS_OS_VER`，不新增额外版本宏，不通过`build_app.sh`或环境变量传递版本。`src/component/cmsis/CMakeLists.txt`读取同一个头文件并只编译当前版本适配源文件；`testsuites/cmsis/CMakeLists.txt`也读取该头文件并选择对应测试目录，保证适配层和测试套版本一致。

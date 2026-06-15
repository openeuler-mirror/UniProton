# UniProton 本地文件系统架构与测试说明

## 1 背景

UniProton 当前文件系统能力主要有三类使用方式：代理文件系统、本地 VFS 文件系统、代理文件系统与本地 VFS 共存。本文关注本地 VFS 文件系统，说明 FATFS、RAMFS、SPIFFS、LITTLEFS、DEVFS 在 UniProton 中的分层关系、宏控开关、差异点和 sd3403 测试映射。

本地文件系统基于 NuttX VFS 相关代码移植。应用侧可以通过标准 C 文件接口或 POSIX fd 接口访问文件和设备，例如 `fopen`、`fwrite`、`fread`、`remove`、`open`、`write`、`read`、`close`。这些接口经过 libc、VFS、inode、mountpoint operation 或 driver operation 后分发到具体文件系统或设备驱动。

当前 sd3403 联调态为了覆盖多文件系统路径，已同时打开 FATFS、RAMFS、SPIFFS、LITTLEFS、DEVFS 相关基础宏。上库后的默认策略应保持这些文件系统宏默认关闭，由用户按需打开并运行对应测试。

## 2 架构图

本地文件系统和设备节点的整体分层如下：

```text
应用 / 测试用例
  |
  | fopen / fwrite / fread / fclose / remove
  | open / write / read / lseek / close
  v
libc stdio / POSIX fd 接口
  |
  | sys_open / sys_write / sys_read / sys_close / unlink / mount
  v
NuttX VFS 适配层
  |
  | fd table、inode tree、路径解析、mountpoint 分发、driver 分发
  +-------------------------------+-------------------------------+
  |                               |                               |
  v                               v                               v
mountpoint 文件系统               字符设备 / 块设备 / MTD 设备       DEVFS 设备节点
  |                               |                               |
  |                               | register_driver               | file_operations
  |                               | register_blockdriver          |
  |                               | register_mtddriver            |
  v                               v                               v
+---------+  +---------+  +----------+  +-----------+        测试用内存字符设备
| FATFS   |  | RAMFS   |  | SPIFFS   |  | LITTLEFS  |
+---------+  +---------+  +----------+  +-----------+
  |            |            |             |
  v            v            v             v
FatFs core   UniProton    SPIFFS       littlefs core
             ramfs        core         + UniProton adapter
  |                         |             |
  v                         v             v
block driver              MTD driver    MTD driver
  |                         |             |
  v                         v             v
内存块设备                 内存 flash    内存 flash
```

## 3 统一层与差异层

### 3.1 统一层

这些层对 FATFS、RAMFS、SPIFFS、LITTLEFS、DEVFS 是一致的：

| 层级 | 共同职责 |
| --- | --- |
| 应用 / 测试用例 | 发起标准文件接口或 fd 接口调用，不直接操作具体文件系统内部结构。 |
| libc / POSIX fd | 将 `FILE` 流或 fd 操作转换为底层 `sys_*` / VFS 调用。 |
| VFS fd 管理 | 管理 fd table、`struct file`、读写偏移和 close 流程。 |
| inode 层 | 维护 `/dev`、挂载点和文件路径对应的 inode 树。 |
| mount 分发 | `mount()` 根据 `filesystemtype` 查找 `g_*_operations` 并绑定 mountpoint。 |
| driver 分发 | `register_driver()`、`register_blockdriver()`、`register_mtddriver()` 注册底层设备节点。 |

### 3.2 差异层

不同文件系统的差异主要集中在设备模型、mount 参数、文件系统 core 和测试接口选择：

| 类型 | fstype / 节点 | 底层设备 | 文件系统 core / 适配层 | 测试接口 |
| --- | --- | --- | --- | --- |
| FATFS | `"vfat"`，`/fat` | block driver：`/dev/ramfat0` | `src/fs/fat/fat_vfsops.c` + FatFs `ff15` + `diskio.c` | `FILE` 流：`fopen` / `fwrite` / `fread` / `remove` |
| RAMFS | `"ramfs"`，`/ram` | 无外部设备，mount data 传入容量 | `src/fs/ramfs/ramfs_vfsops.c` | `FILE` 流：`fopen` / `fwrite` / `fread` / `remove` |
| SPIFFS | `"spiffs"`，`/spiffs` | MTD driver：`/dev/ramspiffs0` | `src/fs/spiffs/spiffs_vfsops.c` + SPIFFS core | `FILE` 流：`fopen` / `fwrite` / `fread` / `remove` |
| LITTLEFS | `"littlefs"`，`/littlefs` | MTD driver：`/dev/ramlittlefs0` | `src/fs/littlefs/littlefs_vfsops.c` + littlefs `v2.9.3` | fd 接口：`open` / `write` / `read` / `remove` |
| DEVFS | `/dev/ramdevfs0` | 字符设备 file operations | VFS driver path，不经过 mountpoint 文件系统 | fd 接口：`open` / `write` / `lseek` / `read` / `close` |

LITTLEFS 测试当前使用 fd 接口，是为了验证 littlefs mountpoint 和 VFS fd 路径，不强制叠加 `FILE` 流封装成本。DEVFS 本质是字符设备节点，也使用 fd 接口验证 driver 分发路径。

## 4 代码接入点

### 4.1 构建入口

`src/fs/CMakeLists.txt` 在 `CONFIG_OS_OPTION_NUTTX_VFS` 打开后接入 VFS 基础层和各文件系统目录：

```text
src/fs/
  adapter/
  inode/
  mount/
  vfs/
  sys/
  fat/
  ramfs/
  littlefs/
  spiffs/
  driver/    # 依赖 CONFIG_OS_OPTION_DRIVER
```

各子目录再按对应文件系统宏决定是否编译：

| 目录 | 主要宏 | 说明 |
| --- | --- | --- |
| `src/fs/fat` | `CONFIG_CONFIG_FS_FAT`，并依赖 `CONFIG_OS_OPTION_DRIVER` | 接入 FatFs 和 block driver glue。 |
| `src/fs/ramfs` | `CONFIG_CONFIG_FS_RAMFS` | 接入 RAMFS。 |
| `src/fs/spiffs` | `CONFIG_CONFIG_FS_SPIFFS` | 复用 SPIFFS core，并通过 UniProton VFS/MTD adapter 本地运行。 |
| `src/fs/littlefs` | `CONFIG_CONFIG_FS_LITTLEFS` | 接入 littlefs core 和 UniProton VFS/MTD adapter。 |
| `src/fs/driver` | `CONFIG_OS_OPTION_DRIVER` | 提供字符设备、块设备、MTD 设备注册和查找能力。 |

文件系统构建必须能在 UniProton 仓库内独立完成，不允许依赖工作区中的兄弟仓库目录。当前外部 core 的处理方式如下：

| 文件系统 | core 来源策略 |
| --- | --- |
| FATFS | `src/fs/fat/CMakeLists.txt` 可通过 `FetchContent` 下载 FatFs 到本仓 `src/fs/fat/ff15`，后续构建使用该本地目录。 |
| SPIFFS | SPIFFS core 已放在本仓 `src/fs/spiffs/core/`，构建时不引用仓库外目录。 |
| LITTLEFS | `src/fs/littlefs/CMakeLists.txt` 可通过 `FetchContent` 下载 littlefs 到本仓 `src/fs/littlefs/littlefs`，后续构建使用该本地目录。 |

### 4.2 mount 类型映射

`src/fs/mount/fs_mount.c` 根据设备类型把 `filesystemtype` 映射到具体 mountpoint operations：

| 设备类型 | fstype | operations |
| --- | --- | --- |
| block driver | `"vfat"` | `g_fat_operations` |
| block driver 或 MTD driver | `"littlefs"` | `g_littlefs_operations` |
| MTD driver | `"spiffs"` | `g_spiffs_operations` |
| 无底层设备 | `"ramfs"` | `g_ramfs_operations` |

DEVFS 不通过 `mount()` 选择 fstype，而是通过 `register_driver()` 在 `/dev` 下注册字符设备 inode。

### 4.3 用户自有存储接入与挂载

当前 sd3403 测试使用的是内存模拟设备，目的是在没有真实 SD、eMMC、NOR、NAND 的情况下验证 VFS、mountpoint、driver 和文件系统 core 的调用链。测试里的设备名如 `/dev/ramfat0`、`/dev/ramspiffs0`、`/dev/ramlittlefs0`、`/dev/ramdevfs0` 不是固定要求，用户接入实际产品时应替换为自己的设备驱动和设备节点。

用户接入已有文件系统时，通常分三步：

1. 打开对应宏控。
2. 注册自己的底层设备节点。
3. 调用 `mount()` 把设备挂到目标目录。

不同文件系统需要的底层设备类型不同：

| 文件系统 | 用户需要提供的底层能力 | 注册接口 | mount 示例 |
| --- | --- | --- | --- |
| FATFS | 块设备，提供 sector read/write/ioctl 能力。 | `register_blockdriver("/dev/mmcblk0", ...)` | `mount("/dev/mmcblk0", "/fat", "vfat", 0, "0:")` |
| SPIFFS | MTD 设备，提供 erase/read/write/geometry 能力。 | `register_mtddriver("/dev/spiflash0", ...)` | `mount("/dev/spiflash0", "/spiffs", "spiffs", 0, NULL)` |
| LITTLEFS | MTD 设备，提供 erase/read/write/geometry 能力。 | `register_mtddriver("/dev/spiflash1", ...)` | `mount("/dev/spiflash1", "/littlefs", "littlefs", 0, NULL)` |
| RAMFS | 不需要外部设备，容量通过 mount data 传入。 | 不需要注册设备 | `mount(NULL, "/ram", "ramfs", 0, (const void *)capacity)` |
| DEVFS | 字符设备 file operations。 | `register_driver("/dev/uart0", ...)` | 不需要 `mount()`，注册后直接访问 `/dev/uart0`。 |

如果用户已有新的文件系统 core，接入方式与现有 FATFS/SPIFFS/LITTLEFS 类似：

1. 在 `src/fs/<newfs>/` 下增加 VFS mountpoint adapter，实现 `struct mountpt_operations`。
2. 在 `src/fs/mount/fs_mount.c` 中把新的 `filesystemtype` 字符串映射到该 operations，例如 `{ "newfs", &g_newfs_operations }`。
3. 在 `src/fs/<newfs>/CMakeLists.txt` 中按独立宏控决定是否编译，并在 `src/fs/CMakeLists.txt` 中加入子目录。
4. 如果文件系统依赖块设备，走 `register_blockdriver()`；如果依赖 flash/MTD，走 `register_mtddriver()`；如果只是字符设备节点，走 `register_driver()`，通常不需要 mountpoint 文件系统。
5. 增加对应测试目录 `testsuites/filesystem-test/<newfs>/`，并在 `filesystem_test.c` 的统一入口中按宏控调度。

关键原则是：VFS 只认识设备节点、fstype 和 mountpoint operations。真实介质由用户自己的 driver 提供，当前测试中的内存设备只用于验证软件栈，不限制最终产品必须使用内存模拟介质。

## 5 宏控说明

### 5.1 基础宏

| defconfig 宏 | 生成宏 | 作用 | 何时需要 |
| --- | --- | --- | --- |
| `CONFIG_OS_OPTION_NUTTX_VFS=y` | `OS_OPTION_NUTTX_VFS` | 打开本地 NuttX VFS、inode、fd、mount、syscall 适配层。 | 使用任一本地文件系统或 DEVFS 时需要。 |
| `CONFIG_OS_OPTION_DRIVER=y` | `OS_OPTION_DRIVER` | 打开 driver 框架，提供 `register_driver()`、`register_blockdriver()`、`register_mtddriver()`。 | 使用 FATFS、SPIFFS、LITTLEFS、DEVFS 时需要。RAMFS 单独测试不依赖外部设备，但通常和 VFS driver 能力一起验证。 |
| `CONFIG_CONFIG_FILE_STREAM=y` | `CONFIG_FILE_STREAM` | 打开 `FILE` 文件流支持。 | 使用 FATFS/RAMFS/SPIFFS 的 `FsFileOps()` 测试时需要。 |
| `CONFIG_CONFIG_MTD=y` | `CONFIG_MTD` | 打开 MTD 设备模型。 | SPIFFS/LITTLEFS 使用 MTD 设备时需要。 |
| `CONFIG_CONFIG_MTD_BYTE_WRITE=y` | `CONFIG_MTD_BYTE_WRITE` | 允许 MTD byte write operation。 | 当前内存 flash 测试和 SPIFFS/LITTLEFS 适配需要。 |
| `CONFIG_CONFIG_NFILE_DESCRIPTORS_PER_BLOCK=<n>` | `CONFIG_NFILE_DESCRIPTORS_PER_BLOCK` | 配置 VFS fd table block 大小。 | 使用文件或设备 fd 时需要有效值。 |
| `# CONFIG_OS_OPTION_PROXY is not set` | 不生成 `OS_OPTION_PROXY` | 避免文件访问被代理到 Linux 侧。 | 验证本地 VFS/FS 路径时应关闭。 |

### 5.2 文件系统宏

| defconfig 宏 | 生成宏 | 启用内容 | 对应测试 |
| --- | --- | --- | --- |
| `CONFIG_CONFIG_FS_FAT=y` | `CONFIG_FS_FAT` | FATFS mountpoint、FatFs core、diskio glue。 | `FatfsTest()` |
| `CONFIG_CONFIG_FS_RAMFS=y` | `CONFIG_FS_RAMFS` | RAMFS mountpoint。 | `RamfsTest()` |
| `CONFIG_CONFIG_FS_SPIFFS=y` | `CONFIG_FS_SPIFFS` | SPIFFS mountpoint、SPIFFS core、MTD glue。 | `SpiffsTest()` |
| `CONFIG_CONFIG_FS_LITTLEFS=y` | `CONFIG_FS_LITTLEFS` | LITTLEFS mountpoint、littlefs core、MTD glue。 | `LittlefsTest()` |

DEVFS 没有单独文件系统宏，依赖 `CONFIG_OS_OPTION_NUTTX_VFS` 和 `CONFIG_OS_OPTION_DRIVER` 注册 `/dev` 字符设备节点。

### 5.3 默认策略

sd3403 当前联调 defconfig 同时打开了多文件系统宏，是为了在一块板上一次性验证完整路径。上库时建议保持以下策略：

| 场景 | 建议 |
| --- | --- |
| 默认配置 | `CONFIG_CONFIG_FS_FAT`、`CONFIG_CONFIG_FS_RAMFS`、`CONFIG_CONFIG_FS_SPIFFS`、`CONFIG_CONFIG_FS_LITTLEFS` 默认关闭。 |
| 用户需要本地文件系统 | 先打开 `CONFIG_OS_OPTION_NUTTX_VFS`，按需打开 `CONFIG_OS_OPTION_DRIVER`、`CONFIG_CONFIG_FILE_STREAM`、`CONFIG_CONFIG_MTD`。 |
| 用户只需要 RAMFS | 打开 `CONFIG_OS_OPTION_NUTTX_VFS` 和 `CONFIG_CONFIG_FS_RAMFS`，如果要跑当前统一测试仍需打开 `CONFIG_CONFIG_FILE_STREAM`。 |
| 用户需要 SPIFFS/LITTLEFS | 打开 `CONFIG_OS_OPTION_DRIVER`、`CONFIG_CONFIG_MTD`、`CONFIG_CONFIG_MTD_BYTE_WRITE` 和对应 FS 宏。 |
| 验证本地路径 | 关闭 `CONFIG_OS_OPTION_PROXY`，避免代理文件系统影响测试结论。 |

## 6 测试套设计

### 6.1 目录与入口

当前统一测试源码位于：

```text
testsuites/filesystem-test/
  CMakeLists.txt
  filesystem_test.c
  filesystem_test.h
  devfs/
    devfs.c
  fatfs/
    fatfs.c
  littlefs/
    littlefs.c
  ramfs/
    ramfs.c
  spiffs/
    spiffs.c
```

`filesystem_test.c` 是统一入口和调度层，各文件系统测试按类型放在独立子目录中。`fatfs/fatfs.c` 只包含 FATFS 测试，不再混放其他文件系统测试。

对外入口为：

```c
void filesystem_test(void);
```

sd3403 的 `demos/sd3403/apps/openamp/main.c` 在 `FILESYSTEM_TESTCASE` 下声明并调用 `filesystem_test()`。调用位置放在 OpenAMP 初始化和 `is_tty_ready()` 完成之后，保证 rpmsg tty ready 后再输出测试日志。

### 6.2 构建接入

`testsuites/filesystem-test/CMakeLists.txt` 仅在 `APP=fs` 时接入统一文件系统测试：

```cmake
if (${APP} STREQUAL "fs")
    set(ALL_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/filesystem_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/fatfs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ramfs/ramfs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/spiffs/spiffs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/littlefs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/devfs/devfs.c
    )
else()
    return()
endif()
```

sd3403 demo 的 `demos/sd3403/CMakeLists.txt` 在 `APP=fs` 时加入 `filesystemTest` object library，并给 `rpmsg` 增加 `FILESYSTEM_TESTCASE` 编译宏。sd3403 文件系统测试只保留统一入口和统一 app 名称。

### 6.3 测试矩阵

测试源码内部按生成后的宏定义选择子测试。未满足依赖宏时，对应子测试不会编译具体文件系统操作，只打印 skip 日志并继续执行其他已开启的子测试。这样 `APP=fs` 可以用于“只开启部分文件系统”的配置，而不强制要求 FATFS、RAMFS、SPIFFS、LITTLEFS 全部开启。

| 子测试 | 宏控要求 | 注册 / mount | 访问路径 | 覆盖内容 |
| --- | --- | --- | --- | --- |
| `FatfsTest()` | `OS_OPTION_NUTTX_VFS`、`OS_OPTION_DRIVER`、`CONFIG_FS_FAT`、`CONFIG_FILE_STREAM` | `register_blockdriver("/dev/ramfat0")`，`mount(..., "/fat", "vfat", ..., "0:")` | `/fat/test.txt` | 空白内存块设备自动格式化、FatFs mount/remount、stdio 文件读写删除、block driver read/write/ioctl。 |
| `RamfsTest()` | `OS_OPTION_NUTTX_VFS`、`CONFIG_FS_RAMFS`、`CONFIG_FILE_STREAM` | `mount(NULL, "/ram", "ramfs", ..., RAMFS_CAPACITY)` | `/ram/test.txt` | 无设备 mountpoint、内存文件创建、stdio 文件读写删除。 |
| `SpiffsTest()` | `OS_OPTION_NUTTX_VFS`、`OS_OPTION_DRIVER`、`CONFIG_MTD`、`CONFIG_MTD_BYTE_WRITE`、`CONFIG_FS_SPIFFS`、`CONFIG_FILE_STREAM` | `register_mtddriver("/dev/ramspiffs0")`，`mount(..., "/spiffs", "spiffs")` | `/spiffs/test.txt` | MTD flash 模拟、SPIFFS mount、stdio 文件读写删除。 |
| `LittlefsTest()` | `OS_OPTION_NUTTX_VFS`、`OS_OPTION_DRIVER`、`CONFIG_MTD`、`CONFIG_MTD_BYTE_WRITE`、`CONFIG_FS_LITTLEFS` | `register_mtddriver("/dev/ramlittlefs0")`，`mount(..., "/littlefs", "littlefs")` | `/littlefs/test.txt` | MTD flash 模拟、littlefs mount、fd 文件读写删除、路径归一化。 |
| `DevfsTest()` | `OS_OPTION_NUTTX_VFS`、`OS_OPTION_DRIVER` | `register_driver("/dev/ramdevfs0")` | `/dev/ramdevfs0` | 字符设备 open/write/lseek/read/close 和 driver operation 分发。 |

如果所有 `FS_ENABLE_*_TEST` 都未满足，`filesystem_test()` 会输出 `[FS][ERROR] no filesystem tests enabled` 并直接返回。

### 6.4 执行流程

统一测试执行顺序如下：

```text
filesystem_test()
  |
  v
fs_initialize()
  |
  +--> FatfsTest()
  |
  +--> RamfsTest()
  |
  +--> SpiffsTest()
  |
  +--> LittlefsTest()
  |
  +--> DevfsTest()
  |
  v
打印 [FS][INFO] sd3403 filesystem tests success 或 failed
```

测试成功时关键日志如下：

```text
[FS][INFO] start sd3403 filesystem tests
[FATFS][INFO] sd3403 memory fatfs test success
[RAMFS][INFO] sd3403 ramfs test success
[SPIFFS][INFO] sd3403 memory spiffs test success
[LITTLEFS][INFO] sd3403 memory littlefs test success
[DEVFS][INFO] sd3403 devfs test success
[FS][INFO] sd3403 filesystem tests success
```

## 7 构建与上板运行

在 Docker 容器内先构建 sd3403 内核库：

```bash
cd /home/uniproton/fs_UniProton/UniProton
python3 build.py sd3403
```

再构建统一文件系统测试 app：

```bash
cd /home/uniproton/fs_UniProton/UniProton/demos/sd3403/build
sh build_app.sh fs
```

构建产物：

```text
demos/sd3403/build/fs.elf
demos/sd3403/build/fs.bin
```

通过 mica 运行时，`ClientPath` 应指向：

```text
/etc/mica/fs.elf
```

本轮 sd3403 板级验证串口日志已追加到：

```text
demos/sd3403/build/test-results.txt
```

最终验证结论：FATFS、RAMFS、SPIFFS、LITTLEFS、DEVFS 均通过，并输出 `[FS][INFO] sd3403 filesystem tests success`。

## 8 覆盖范围与限制

当前统一测试覆盖：

- 本地 VFS 初始化、fd 管理、inode 查找、mountpoint operation 分发。
- FATFS 到 block driver 的读写和 ioctl 路径。
- RAMFS 无设备 mountpoint 的文件读写路径。
- SPIFFS/LITTLEFS 到 MTD driver 的读、写、擦除、geometry 路径。
- DEVFS 字符设备注册和 file operations 分发路径。
- FATFS/RAMFS/SPIFFS 的 libc `FILE` 流路径。
- LITTLEFS/DEVFS 的 POSIX fd 路径。

当前统一测试不覆盖：

- 真实 SD/eMMC/NOR/NAND 等存储介质驱动。
- 掉电恢复、磨损均衡、坏块管理和长时间压力写入。
- 多任务并发文件读写一致性。
- 文件系统卸载后的资源回收完整性。
- FATFS/littlefs core 首次构建时可能需要网络下载到本仓本地目录；离线构建前需确保 `src/fs/fat/ff15` 和 `src/fs/littlefs/littlefs` 已存在。

## 9 相关文件

| 文件 | 说明 |
| --- | --- |
| `src/fs/CMakeLists.txt` | 本地 VFS 和各文件系统构建入口。 |
| `src/fs/mount/fs_mount.c` | fstype 到 mountpoint operations 的映射。 |
| `src/fs/fat/fat_vfsops.c` | FATFS VFS 适配层。 |
| `src/fs/fat/diskio.c` | FatFs 到 block driver 的 glue。 |
| `src/fs/ramfs/ramfs_vfsops.c` | RAMFS 实现。 |
| `src/fs/spiffs/core/` | SPIFFS core 源码，随 UniProton 本地文件系统一起构建。 |
| `src/fs/spiffs/spiffs_vfsops.c` | SPIFFS VFS/MTD 适配层。 |
| `src/fs/littlefs/littlefs_vfsops.c` | LITTLEFS VFS/MTD 适配层。 |
| `testsuites/filesystem-test/filesystem_test.c` | sd3403 统一多文件系统测试入口和调度。 |
| `testsuites/filesystem-test/filesystem_test.h` | 各子测试共享的宏控判断和函数声明。 |
| `testsuites/filesystem-test/fatfs/fatfs.c` | FATFS 测试源码。 |
| `testsuites/filesystem-test/ramfs/ramfs.c` | RAMFS 测试源码。 |
| `testsuites/filesystem-test/spiffs/spiffs.c` | SPIFFS 测试源码。 |
| `testsuites/filesystem-test/littlefs/littlefs.c` | LITTLEFS 测试源码。 |
| `testsuites/filesystem-test/devfs/devfs.c` | DEVFS 测试源码。 |
| `demos/sd3403/apps/openamp/main.c` | `FILESYSTEM_TESTCASE` 运行入口。 |
| `build/uniproton_config/config_armv8_sd3403/defconfig` | sd3403 联调态宏控配置。 |
| `demos/sd3403/build/test-results.txt` | 本轮板级串口验证日志。 |

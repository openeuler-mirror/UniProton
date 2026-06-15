# UniProton 文件系统分层架构与 FATFS 测试套说明

## 1 背景

UniProton 当前文件系统能力主要有三类使用方式：代理文件系统、本地 VFS 文件系统、代理文件系统与本地 VFS 共存。本文关注本地 VFS 文件系统，以及新增的 `testsuites/filesystem-test/fatfs` 测试套如何与现有架构组合。

本地 VFS 文件系统基于 NuttX VFS 相关代码移植，目前 FAT 文件系统通过 FatFs 接入。应用侧可以通过标准 C 文件接口访问文件，例如 `fopen`、`fwrite`、`fread`、`remove`。这些接口经过 libc、VFS、mountpoint operation、FatFs glue、块设备驱动后完成实际读写。

## 2 FATFS 测试所需宏控

sd3403 FATFS 测试依赖 `build/uniproton_config/config_armv8_sd3403/defconfig` 中的功能宏。构建时这些宏会生成到 `prt_buildef.h`，并进一步控制内核静态库、demo 头文件和 CMake 子目录是否包含对应模块。

### 2.1 必须开启的文件系统相关宏

| defconfig 宏 | 生成宏 | 是否必须 | 原因 |
| --- | --- | --- | --- |
| `CONFIG_OS_OPTION_NUTTX_VFS=y` | `OS_OPTION_NUTTX_VFS` | 是 | 打开本地 NuttX VFS 代码。没有该宏，`src/fs` 中 VFS、mount、inode、fd 管理等本地文件系统路径不会完整编译，`mount()`、`register_blockdriver()` 和本地 `sys_*` 文件操作路径不可用。 |
| `CONFIG_OS_OPTION_DRIVER=y` | `OS_OPTION_DRIVER` | 是 | 打开 driver 框架，提供块设备注册和查找能力。FATFS 测试需要 `register_blockdriver("/dev/ramfat0", ...)` 注册内存块设备，`mount()` 再通过 `find_blockdriver()` 找到它。 |
| `CONFIG_CONFIG_FS_FAT=y` | `CONFIG_FS_FAT` | 是 | 打开 FAT 文件系统支持，编译 `src/fs/fat` 下的 FAT VFS 层、FatFs glue 和相关源码。没有该宏，`mount(..., "vfat", ...)` 找不到 FAT mountpoint operation。 |
| `CONFIG_CONFIG_FILE_STREAM=y` | `CONFIG_FILE_STREAM` | 是 | 打开基于 `FILE` 的 stdio 文件流支持。测试要求使用 `fopen`、`fwrite`、`fread`、`remove` 等标准接口验证，缺少该宏时 `FILE` 流相关结构和接口路径不完整。 |
| `CONFIG_CONFIG_NFILE_DESCRIPTORS_PER_BLOCK=6` | `CONFIG_NFILE_DESCRIPTORS_PER_BLOCK` | 是 | 配置 VFS fd table 每个 block 的 fd 数量。标准文件接口最终会分配 fd，必须有 fd 管理能力。该值不要求一定为 6，但必须配置为有效值。 |
| `CONFIG_CONFIG_DISABLE_ENVIRON=y` | `CONFIG_DISABLE_ENVIRON` | 建议保持 | 当前 sd3403 freestanding 环境不依赖进程环境变量。保持关闭 environ 能减少不需要的环境变量支持路径，和现有 NuttX VFS 配置保持一致。 |

### 2.2 sd3403 测试场景需要保留的宏

| defconfig 宏 | 是否必须 | 原因 |
| --- | --- | --- |
| `CONFIG_OS_OPTION_OPENAMP=y` | 对 sd3403 demo 必须 | sd3403 demo 通过 OpenAMP/MICA 拉起并创建 rpmsg tty。当前 `main.c` 中 FATFS 测试放在 `is_tty_ready()` 完成之后执行，因此需要保留 OpenAMP 初始化路径。 |
| `CONFIG_OS_OPTION_POSIX=y` | 建议保持 | 当前 libc、pthread mutex、stdio 锁等路径依赖 POSIX 适配能力。FATFS 测试本身不直接测试 POSIX API，但标准 C 文件流和底层锁实现会使用相关基础设施。 |
| `CONFIG_OS_OPTION_TICK=y` | 对当前 demo 必须 | `main.c` 等待 `is_tty_ready()` 时使用 `PRT_TaskDelay()`，需要 tick 驱动任务延时。 |

### 2.3 必须关闭或不建议开启的宏

| defconfig 宏 | 生成状态 | 原因 |
| --- | --- | --- |
| `# CONFIG_OS_OPTION_PROXY is not set` | 不生成 `OS_OPTION_PROXY` | 本测试验证本地 VFS + FATFS 路径，不能走代理文件系统。开启 proxy 后，部分文件路径可能被代理层截获，测试结果不能证明本地 FATFS 完整链路。 |
| `# CONFIG_CONFIG_NET is not set` | 不生成 `CONFIG_NET` | 本测试不验证网络文件或 socket 路径。关闭网络总开关后，本地 FATFS 测试不依赖网络模块；`CONFIG_CONFIG_NET_SENDFILE` 这类 VFS sendfile 细分开关可能仍保留，但不参与本次 `fopen` / `fwrite` / `fread` / `remove` 验证路径。 |

### 2.4 宏控与调用链的关系

FATFS 测试从标准文件接口到内存块设备的依赖关系如下：

```text
CONFIG_CONFIG_FILE_STREAM
  -> fopen / fwrite / fread / fclose 的 FILE 流支持

CONFIG_OS_OPTION_NUTTX_VFS
  -> sys_open / sys_write / sys_read / sys_close / mount / inode / fd table

CONFIG_OS_OPTION_DRIVER
  -> register_blockdriver / find_blockdriver / block_operations

CONFIG_CONFIG_FS_FAT
  -> vfat mountpoint operations / FatFs / diskio glue

# CONFIG_OS_OPTION_PROXY is not set
  -> 文件操作不走代理文件系统，直接进入本地 VFS
```

如果上述任一必需宏缺失，测试可能表现为编译缺符号、`mount()` 失败、`fopen()` 返回失败，或者文件操作没有进入本地 FATFS 路径。

## 3 本地文件系统分层

本地 FATFS 文件系统的主要调用链如下：

```text
应用 / 测试用例
  |
  | fopen / fwrite / fread / fclose / remove
  v
libc stdio 层
  |
  | __fdopen / __stdio_write / __stdio_read / __stdio_close
  v
VFS syscall 适配层
  |
  | sys_open / sys_write / sys_read / sys_close / remove
  v
NuttX VFS 层
  |
  | inode 查找、文件描述符管理、mountpoint operation 分发
  v
FAT VFS mountpoint 层
  |
  | fat_open / fat_write / fat_read / fat_close / fat_unlink
  v
FatFs 文件系统核心
  |
  | f_mount / f_mkfs / f_open / f_write / f_read / f_close / f_unlink
  v
FatFs diskio glue
  |
  | disk_read / disk_write / disk_ioctl
  v
块设备驱动
  |
  | block_operations: read / write / geometry / ioctl
  v
实际存储介质或测试用内存盘
```

## 4 各层职责

### 4.1 应用与测试用例层

应用或测试用例只使用标准文件接口，不直接调用 `sys_*` 接口做功能验证。这样验证的是从 libc 到 VFS 到 FATFS 到块设备的完整路径，而不是绕过 libc 的局部路径。

FATFS 测试套位于：

```text
testsuites/filesystem-test/fatfs/fatfs.c
```

导出入口为：

```c
void fatfs_test(void);
```

该入口由 sd3403 demo 在 `FILESYSTEM_TESTCASE` 宏开启时调用。

### 4.2 libc stdio 层

标准 C 文件接口由 libc 提供。例如：

- `fopen()` 打开文件并创建 `FILE` 对象。
- `fwrite()` 先写入 stdio buffer。
- `fclose()` 会先 `fflush()` 刷新写缓冲，再关闭底层 fd。
- `fread()` 从底层 fd 读取到 stdio buffer。

这些接口最终会调用 VFS 提供的 `sys_open`、`sys_write`、`sys_read`、`sys_close` 等接口。

### 4.3 VFS syscall 适配层

VFS syscall 适配层负责把 libc 层传入的 fd、buffer、flags 转换成内部 `struct file` 操作。典型文件包括：

```text
src/fs/vfs/fs_open.c
src/fs/vfs/fs_write.c
src/fs/vfs/fs_read.c
src/fs/vfs/fs_close.c
src/fs/inode/fs_files.c
```

这一层不理解具体文件系统格式，只负责 fd 管理、权限检查、inode 获取和调用对应 inode operation。

### 4.4 VFS inode 与 mount 层

NuttX VFS 使用 inode 树表示路径、设备节点和挂载点。块设备通过 `register_blockdriver()` 注册成 inode，文件系统通过 `mount()` 绑定到 mountpoint inode。

FATFS 测试套使用：

```c
register_blockdriver("/dev/ramfat0", &g_fatfsRamBops, 0, &g_fatfsRamDev);
mount("/dev/ramfat0", "/", "vfat", 0, "0:");
```

其中 `/dev/ramfat0` 是测试套注册的内存块设备，`/` 是挂载点，`vfat` 选择 FAT mountpoint operations，`0:` 是 FatFs 使用的 volume 名称。

### 4.5 FAT VFS mountpoint 层

FAT VFS 层位于：

```text
src/fs/fat/fat_vfsops.c
```

它把 VFS mountpoint operation 转换为 FatFs API。例如：

| VFS operation | FatFs API |
| --- | --- |
| `fat_bind` | `f_mount` / `f_mkfs` |
| `fat_open` | `f_open` |
| `fat_write` | `f_write` |
| `fat_read` | `f_read` |
| `fat_close` | `f_close` |
| `fat_unlink` | `f_unlink` |

这一层还维护 `struct fat_mountpt_s`，保存 FatFs `FATFS` 对象、底层块设备 inode 和文件系统锁。

### 4.6 FatFs diskio glue 层

FatFs 核心通过 `disk_read`、`disk_write`、`disk_ioctl` 访问底层设备。UniProton 的 glue 代码位于：

```text
src/fs/fat/diskio.c
```

该层根据 FatFs volume 号找到注册过的块设备 inode，再调用 `struct block_operations` 中的 `read`、`write`、`geometry`、`ioctl`。

### 4.7 块设备层

块设备由 `register_blockdriver()` 注册。测试套中的内存盘提供了如下 block operations：

```c
static const struct block_operations g_fatfsRamBops = {
    NULL,
    NULL,
    FatfsRamRead,
    FatfsRamWrite,
    FatfsRamGeometry,
    FatfsRamIoctl,
    NULL,
};
```

测试盘使用一段静态内存模拟扇区：

```c
#define FATFS_SECTOR_SIZE 512
#define FATFS_SECTOR_COUNT 2048

static unsigned char g_fatfsRamDisk[FATFS_SECTOR_SIZE * FATFS_SECTOR_COUNT];
```

FatFs 对该块设备执行 `read`、`write`、`ioctl(BIOC_PARTINFO)` 和 `ioctl(BIOC_FLUSH)`，测试套把这些操作映射到内存读写。

## 5 FATFS 测试套设计

### 5.1 目录结构

新增测试套目录为：

```text
testsuites/filesystem-test/
  CMakeLists.txt
  fatfs/
    fatfs.c
```

这样后续增加其他文件系统测试时，可以继续扩展为：

```text
testsuites/filesystem-test/
  littlefs/
  ext2/
  ...
```

### 5.2 构建接入

`testsuites/filesystem-test/CMakeLists.txt` 根据 `APP` 选择 FATFS 测试源码：

```cmake
if (${APP} STREQUAL "fatfs")
    set(ALL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fatfs/fatfs.c)
else()
    return()
endif()

if (${CPU_TYPE} STREQUAL "sd3403")
    add_library(filesystemTest OBJECT ${ALL_SRC})
endif()
```

sd3403 demo 的 `CMakeLists.txt` 增加 `APP == "fatfs"` 分支，把测试套编译成 object library 并链接到最终 ELF：

```cmake
elseif (${APP} STREQUAL "fatfs")
    add_subdirectory(${HOME_PATH}/testsuites/filesystem-test tmp)
    target_compile_options(rpmsg PUBLIC -DFILESYSTEM_TESTCASE)
    list(APPEND OBJS
        $<TARGET_OBJECTS:rpmsg>
        $<TARGET_OBJECTS:bsp>
        $<TARGET_OBJECTS:config>
        $<TARGET_OBJECTS:filesystemTest>)
```

### 5.3 运行入口

测试套参考 SOEM 测试方式，不使用通用 `Init()` 作为入口，而是导出明确的文件系统测试入口：

```c
void fatfs_test(void);
```

sd3403 的 `demos/sd3403/apps/openamp/main.c` 在 `FILESYSTEM_TESTCASE` 下声明并调用：

```c
#if defined(FILESYSTEM_TESTCASE)
void fatfs_test(void);
#endif
```

调用位置放在 OpenAMP 初始化和 `is_tty_ready()` 完成之后：

```c
#if defined(OS_OPTION_OPENAMP)
    while (!is_tty_ready()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
#endif

#if defined(FILESYSTEM_TESTCASE)
    fatfs_test();
#endif
```

这样测试执行时，OpenAMP rpmsg tty 已经 ready，和 sd3403 其他 app 的启动顺序保持一致。

### 5.4 测试流程

FATFS 测试套执行流程如下：

```text
fatfs_test()
  |
  v
fs_initialize()
  |
  v
清零内存盘
  |
  v
register_blockdriver("/dev/ramfat0", ...)
  |
  v
mount("/dev/ramfat0", "/", "vfat", 0, "0:")
  |
  v
fopen("/test.txt", "r") 预期失败
  |
  v
fopen("/test.txt", "w")
  |
  v
fwrite("hello sd3403 fatfs!")
  |
  v
fclose()
  |
  v
fopen("/test.txt", "r")
  |
  v
fread() 并校验内容
  |
  v
remove("/test.txt")
```

测试成功时输出：

```text
[FATFS][INFO] start sd3403 memory fatfs test
[FATFS][INFO] register block driver success
[FATFS][INFO] mount success
[FATFS][INFO] read from file: hello sd3403 fatfs!
[FATFS][INFO] sd3403 memory fatfs test success
```

### 5.5 测试覆盖范围

该测试覆盖完整本地 FATFS 调用链：

- libc stdio 的 `fopen`、`fwrite`、`fread`、`fclose`、`remove`。
- VFS fd 管理和 mountpoint operation 分发。
- FAT VFS 层的 open/write/read/close/unlink。
- FatFs 的 mount、mkfs、file operation 和 sync 流程。
- FatFs diskio 到 block driver 的 read/write/ioctl 路径。

该测试不覆盖真实存储介质驱动、电源掉电恢复、wear leveling、坏块管理或持久化介质一致性。它的目标是验证 UniProton 本地 VFS + FATFS 软件栈自身能在标准文件 API 下正确工作。

## 6 构建与运行

在 Docker 容器内构建 sd3403 FATFS 测试 app：

```bash
cd /home/uniproton/fs_UniProton/UniProton/demos/sd3403/build
sh build_app.sh fatfs
```

构建产物：

```text
demos/sd3403/build/fatfs.elf
demos/sd3403/build/fatfs.bin
```

通过 mica 运行时，`ClientPath` 应指向：

```text
/etc/mica/fatfs.elf
```

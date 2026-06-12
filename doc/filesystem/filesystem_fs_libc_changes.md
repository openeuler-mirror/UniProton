# FATFS 测试相关 fs 与 libc 修改说明

## 1 背景

在 sd3403 上接入本地 FATFS 测试套时，测试使用内存块设备模拟一块空白磁盘，并通过标准 C 文件接口执行 `fopen`、`fwrite`、`fread`、`remove`。该路径触发了本地 VFS + FATFS 的完整调用链，也暴露出几个原有代码在特定路径下的问题。

本文说明本次修改的 `fs` 和 `libc` 代码位置、问题现象、原因以及修改理由。

## 2 相关宏控

这些修改对应的是本地 VFS + FATFS + stdio 文件流路径。只有启用对应宏后，相关代码才会编译或被测试稳定触发。

| 宏 | 作用 | 与本次修改的关系 |
| --- | --- | --- |
| `CONFIG_OS_OPTION_NUTTX_VFS=y` | 启用本地 NuttX VFS | `src/fs/mount/fs_mount.c` 的 mount/inode 路径依赖该宏。FATFS 测试中的 `mount()` 会触发该路径。 |
| `CONFIG_OS_OPTION_DRIVER=y` | 启用 driver 框架 | 测试通过 `register_blockdriver()` 注册内存块设备，`mount()` 通过 block driver 查找设备。 |
| `CONFIG_CONFIG_FS_FAT=y` | 启用 FAT 文件系统 | `src/fs/fat/fat_vfsops.c` 和 FatFs glue 依赖该宏。空白盘首次挂载会触发 `fat_bind()` 中的 `f_mount()` / `f_mkfs()` / remount 路径。 |
| `CONFIG_CONFIG_FILE_STREAM=y` | 启用 `FILE` 文件流 | `fopen`、`fwrite`、`fread`、`fclose` 测试路径依赖该宏。`fclose()` -> `fflush()` 会触发 stdio `FILE` 锁重入问题。 |
| `# CONFIG_OS_OPTION_PROXY is not set` | 关闭代理文件系统 | 保证文件操作走本地 VFS，而不是代理到 Linux 侧。否则无法验证本地 FATFS 软件栈。 |
| `# CONFIG_CONFIG_NET is not set` | 关闭网络总开关 | 本测试不依赖网络文件或 socket 路径。即使 `CONFIG_CONFIG_NET_SENDFILE` 等 VFS sendfile 细分开关保留，也不参与本次标准文件接口到本地 FATFS 的验证路径。 |

如果缺少 `CONFIG_OS_OPTION_NUTTX_VFS`、`CONFIG_OS_OPTION_DRIVER` 或 `CONFIG_CONFIG_FS_FAT`，测试无法完成本地 FATFS mount。缺少 `CONFIG_CONFIG_FILE_STREAM` 时，测试无法通过标准 `FILE` 接口覆盖 libc stdio 路径。

## 3 修改概览

本次涉及的核心修改文件如下：

```text
src/fs/mount/fs_mount.c
src/fs/fat/fat_vfsops.c
src/libc/musl/src/stdio/__lockfile.c
```

对应解决的问题：

| 文件 | 问题 | 修改目的 |
| --- | --- | --- |
| `src/fs/mount/fs_mount.c` | `nx_mount()` 已持有 inode 锁后再次调用会加锁的 `inode_find()` | 避免 mount 过程同线程自锁 |
| `src/fs/fat/fat_vfsops.c` | FAT 自动格式化后 remount 结果被 cleanup 返回值污染 | 只判断真正 remount 的结果 |
| `src/libc/musl/src/stdio/__lockfile.c` | `fclose()` 调用 `fflush()` 时 stdio `FILE` 锁重入自锁 | 支持同线程递归锁计数 |

## 4 `src/fs/mount/fs_mount.c` 修改说明

### 4.1 问题现象

FATFS 测试执行到：

```c
mount("/dev/ramfat0", "/", "vfat", 0, "0:");
```

时，流程进入 `nx_mount()` 后卡住。定位后发现卡在 inode 锁相关路径。

### 4.2 原因

`nx_mount()` 中先调用：

```c
inode_lock();
```

随后在已经持有 inode 锁的情况下，原代码调用：

```c
ret = inode_find(&desc);
```

但 `inode_find()` 本身会再次执行 inode 加锁。sd3403 当前 mutex/sem 实现下，这种同线程重复加锁会造成自锁，mount 流程无法继续。

从语义上看，此处只是要在已经持锁的情况下搜索目标 mountpoint inode。已持锁场景应该使用不重复加锁的搜索函数。

### 4.3 修改内容

将：

```c
ret = inode_find(&desc);
```

修改为：

```c
ret = inode_search(&desc);
```

并在找到 inode 后补充引用计数：

```c
mountpt_inode->i_crefs++;
```

### 4.4 为什么要补 `i_crefs++`

原来的 `inode_find()` 不只是搜索 inode，还会在成功时增加 inode 引用计数。替换成 `inode_search()` 后，它只负责查找，不负责引用计数。因此需要手动补上：

```c
mountpt_inode->i_crefs++;
```

否则 mount 流程后续释放引用时可能破坏 inode 引用计数平衡。

### 4.5 修改影响

该修改只影响 `nx_mount()` 已经持有 inode 锁后的查找路径。它避免了重复加锁，同时保持原 `inode_find()` 成功路径的引用计数语义。

## 5 `src/fs/fat/fat_vfsops.c` 修改说明

### 5.1 问题现象

FATFS 测试使用的内存盘启动时全为 `0`，相当于一块空白磁盘。第一次挂载时：

```c
ret = f_mount(fs->ff_fs, (const TCHAR *)data, 1);
```

返回 `FR_NO_FILESYSTEM`，随后 `fat_bind()` 调用 `f_mkfs()` 自动格式化。格式化成功后，原代码仍然返回失败，导致 `mount()` 失败。

### 5.2 原因

原代码在自动格式化后执行：

```c
ret = f_mount(NULL, (const TCHAR *)data, 1);
ret += f_mount(fs->ff_fs, (const TCHAR *)data, 1);
```

这里有两个问题：

1. 第一次 `f_mount(fs->ff_fs, data, 1)` 已经返回 `FR_NO_FILESYSTEM`，说明该 volume 没有真正完成挂载。
2. 后续 `f_mount(NULL, data, 1)` 只是 cleanup/unmount 动作，可能返回 `FR_NOT_ENABLED`，表示 volume 当前并未启用。

`FR_NOT_ENABLED` 在这里不是真正的 remount 失败，它只是清理一个未启用 volume 的返回值。但原代码把这个返回值与后续真正 remount 的结果相加：

```text
cleanup 返回 FR_NOT_ENABLED(12) + remount 返回 FR_OK(0) = 12
```

最终即使 remount 成功，也被误判为失败。

### 5.3 修改内容

将：

```c
ret = f_mount(NULL, (const TCHAR *)data, 1);
ret += f_mount(fs->ff_fs, (const TCHAR *)data, 1);
```

修改为：

```c
(void)f_mount(NULL, (const TCHAR *)data, 1);
ret = f_mount(fs->ff_fs, (const TCHAR *)data, 1);
```

### 5.4 为什么显式 `(void)` 忽略 cleanup 返回值

这里的 `f_mount(NULL, data, 1)` 是清理动作，不是判断 FATFS 是否已经可用的关键步骤。真正决定 mount 是否成功的是后面的：

```c
ret = f_mount(fs->ff_fs, (const TCHAR *)data, 1);
```

使用 `(void)` 可以明确表达：cleanup 返回值不参与最终错误判断，避免出现“先赋值再覆盖”的无用 `ret`。

### 5.5 对真实磁盘的影响

如果真实磁盘已经存在合法 FAT 文件系统，第一次 `f_mount()` 会直接返回 `FR_OK`，不会进入自动格式化分支，因此不受该修改影响。

如果真实磁盘是空白盘、未格式化盘或文件系统损坏到需要自动格式化的状态，也会进入该分支。此修改同样适用，因为它修正的是自动格式化后的 remount 结果判断逻辑，而不是内存盘专用逻辑。

## 6 `src/libc/musl/src/stdio/__lockfile.c` 修改说明

### 6.1 问题现象

FATFS 测试执行：

```c
fwrite(...);
fclose(fd);
```

时，`fwrite()` 已返回成功，但 `fclose()` 卡住。定位后发现卡在 `fclose()` 调用 `fflush()` 的 stdio 锁路径上。

### 6.2 原因

`fclose()` 的逻辑会先锁住 `FILE`：

```c
FLOCK(f);
r = fflush(f);
```

而 `fflush(f)` 内部也会执行：

```c
FLOCK(f);
```

也就是说，`fclose()` 已经持有同一个 `FILE` 锁后，调用 `fflush()` 又尝试对同一个 `FILE` 加锁。当前 `OS_OPTION_NUTTX_VFS` 下的 `__lockfile()` 直接调用 `pthread_mutex_lock(&f->mutex)`，没有处理同线程重入，导致同线程自锁。

### 6.3 修改内容

在 `OS_OPTION_NUTTX_VFS` 下，让 `__lockfile()` 识别同一 owner 的重复加锁，并使用 `lockcount` 计数：

```c
long self = (long)pthread_self();

if (f->owner == self) {
    if (f->lockcount == LONG_MAX) {
        return 0;
    }
    f->lockcount++;
    return 1;
}

if (pthread_mutex_lock(&f->mutex) == 0) {
    f->owner = self;
    f->lockcount = 1;
    return 1;
}
```

对应地，`__unlockfile()` 先递减重入计数，只有最后一次 unlock 才释放底层 mutex：

```c
if (f->lockcount > 1) {
    f->lockcount--;
    return;
}

f->lockcount = 0;
if (pthread_mutex_unlock(&f->mutex) == 0) {
    f->owner = -1;
}
```

### 6.4 为什么不只改 `fclose()`

只改 `fclose()` 可以绕过当前现象，但问题本质是 `FILE` 锁在 `OS_OPTION_NUTTX_VFS` 下缺少同线程重入处理。

stdio 中还有其他接口可能在已持锁情况下复用内部函数。如果只在 `fclose()` 里特殊处理，后续类似路径仍可能出现同类问题。修复 `__lockfile()` / `__unlockfile()` 是更符合 stdio 锁语义的改法。

### 6.5 与 `flockfile()` / `ftrylockfile()` 的关系

UniProton 的 stdio 结构中已经有 `owner` 和 `lockcount` 字段，`ftrylockfile()` 也包含同 owner 增加 `lockcount` 的逻辑。本次修改让阻塞式 `__lockfile()` 与 `ftrylockfile()` 的重入语义保持一致。

## 7 验证方式

构建 FATFS 测试 app：

```bash
cd /home/uniproton/fs_UniProton/UniProton/demos/sd3403/build
sh build_app.sh fatfs
```

部署 `fatfs.elf` 后通过 mica 运行，成功日志如下：

```text
[openamp] ept ready
[FATFS][INFO] start sd3403 memory fatfs test
[FATFS][INFO] register block driver success
[FATFS][INFO] mount success
[FATFS][INFO] read from file: hello sd3403 fatfs!
[FATFS][INFO] sd3403 memory fatfs test success
```

该日志证明：

- mount 路径不再因为 inode 锁重入卡住。
- 空白内存盘能够自动格式化并 remount 成功。
- `fclose()` 能正常完成 `fflush()` 和底层 close。
- 标准文件接口完整读写删除流程通过。

## 8 注意事项

本次修改不是为了绕过测试，而是修复测试触发的真实路径问题：

- `fs_mount.c` 修复的是已持 inode 锁后重复加锁的问题。
- `fat_vfsops.c` 修复的是 FAT 自动格式化后 remount 返回值判断问题。
- `__lockfile.c` 修复的是 stdio `FILE` 锁同线程重入问题。

这些问题在普通已格式化磁盘路径上不一定触发，但在空白盘首次挂载、stdio 写缓冲关闭等路径上会稳定出现。

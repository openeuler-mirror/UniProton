#include "filesystem_test.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>
#include "nuttx/fs/fs.h"
#include "nuttx/mtd/mtd.h"
#include "securec.h"

#define LITTLEFS_DEV_NAME "/dev/ramlittlefs0"
#define LITTLEFS_MOUNT_POINT "/littlefs"
#define FLASH_BLOCK_SIZE 256
#define FLASH_ERASE_SIZE 4096
#define FLASH_ERASE_BLOCKS 128

extern unsigned int PRT_Printf(const char *format, ...);

#ifdef FS_ENABLE_LITTLEFS_TEST
struct LittlefsRamMtdDev {
    struct mtd_dev_s mtd;
    unsigned char *data;
    size_t size;
    size_t blockSize;
    size_t eraseSize;
    size_t eraseBlocks;
};

static unsigned char g_littlefsRamFlash[FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS];
static struct LittlefsRamMtdDev g_littlefsMtdDev;

static ssize_t LittlefsRamMtdRead(struct mtd_dev_s *mtd, off_t offset, size_t nbytes, unsigned char *buffer)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;

    if (dev == NULL || buffer == NULL || offset < 0 || (size_t)offset + nbytes > dev->size) {
        return -EINVAL;
    }

    (void)memcpy_s(buffer, nbytes, dev->data + offset, nbytes);
    return (ssize_t)nbytes;
}

static ssize_t LittlefsRamMtdWrite(struct mtd_dev_s *mtd, off_t offset, size_t nbytes, const unsigned char *buffer)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;
    size_t i;

    if (dev == NULL || buffer == NULL || offset < 0 || (size_t)offset + nbytes > dev->size) {
        return -EINVAL;
    }

    for (i = 0; i < nbytes; i++) {
        dev->data[offset + i] &= buffer[i];
    }
    return (ssize_t)nbytes;
}

static int LittlefsRamMtdErase(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;

    if (dev == NULL || startblock < 0 || (size_t)startblock + nblocks > dev->eraseBlocks) {
        return -EINVAL;
    }

    (void)memset_s(dev->data + ((size_t)startblock * dev->eraseSize), nblocks * dev->eraseSize,
                   0xff, nblocks * dev->eraseSize);
    return (int)nblocks;
}

static ssize_t LittlefsRamMtdBread(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks, unsigned char *buffer)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;

    if (dev == NULL) {
        return -EINVAL;
    }
    return LittlefsRamMtdRead(mtd, startblock * dev->blockSize, nblocks * dev->blockSize, buffer) < 0 ? -EINVAL :
           (ssize_t)nblocks;
}

static ssize_t LittlefsRamMtdBwrite(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks,
                                    const unsigned char *buffer)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;

    if (dev == NULL) {
        return -EINVAL;
    }
    return LittlefsRamMtdWrite(mtd, startblock * dev->blockSize, nblocks * dev->blockSize, buffer) < 0 ? -EINVAL :
           (ssize_t)nblocks;
}

static int LittlefsRamMtdIoctl(struct mtd_dev_s *mtd, int cmd, unsigned long arg)
{
    struct LittlefsRamMtdDev *dev = (struct LittlefsRamMtdDev *)mtd;

    if (dev == NULL) {
        return -EINVAL;
    }

    if (cmd == MTDIOC_GEOMETRY) {
        struct mtd_geometry_s *geo = (struct mtd_geometry_s *)arg;
        if (geo == NULL) {
            return -EINVAL;
        }
        (void)memset_s(geo, sizeof(*geo), 0, sizeof(*geo));
        geo->blocksize = dev->blockSize;
        geo->erasesize = dev->eraseSize;
        geo->neraseblocks = dev->eraseBlocks;
        return 0;
    }

    if (cmd == MTDIOC_BULKERASE) {
        (void)memset_s(dev->data, dev->size, 0xff, dev->size);
        return 0;
    }

    return -ENOTTY;
}

static void LittlefsRamMtdInit(struct LittlefsRamMtdDev *dev, unsigned char *data, const char *name)
{
    (void)memset_s(dev, sizeof(*dev), 0, sizeof(*dev));
    (void)memset_s(data, FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS, 0xff, FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS);
    dev->mtd.erase = LittlefsRamMtdErase;
    dev->mtd.bread = LittlefsRamMtdBread;
    dev->mtd.bwrite = LittlefsRamMtdBwrite;
    dev->mtd.read = LittlefsRamMtdRead;
    dev->mtd.write = LittlefsRamMtdWrite;
    dev->mtd.ioctl = LittlefsRamMtdIoctl;
    dev->mtd.name = name;
    dev->data = data;
    dev->size = FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS;
    dev->blockSize = FLASH_BLOCK_SIZE;
    dev->eraseSize = FLASH_ERASE_SIZE;
    dev->eraseBlocks = FLASH_ERASE_BLOCKS;
}

static int LittlefsFdOps(const char *path)
{
    int fd;
    char wrStr[] = "hello sd3403 LITTLEFS!";
    char rdStr[sizeof(wrStr)] = {0};
    int ret;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        PRT_Printf("[LITTLEFS][ERROR] open non-existent file with read success\n");
        (void)close(fd);
        return -1;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        PRT_Printf("[LITTLEFS][ERROR] open file for write failed\n");
        return -1;
    }
    if (write(fd, wrStr, sizeof(wrStr)) != (ssize_t)sizeof(wrStr)) {
        PRT_Printf("[LITTLEFS][ERROR] write file failed\n");
        (void)close(fd);
        return -1;
    }
    (void)close(fd);

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        PRT_Printf("[LITTLEFS][ERROR] open file for read failed\n");
        return -1;
    }
    if (read(fd, rdStr, sizeof(rdStr)) != (ssize_t)sizeof(rdStr)) {
        PRT_Printf("[LITTLEFS][ERROR] read file failed\n");
        (void)close(fd);
        return -1;
    }
    (void)close(fd);

    if (strcmp(wrStr, rdStr) != 0) {
        PRT_Printf("[LITTLEFS][ERROR] read data mismatch: %s\n", rdStr);
        return -1;
    }

    PRT_Printf("[LITTLEFS][INFO] read from file: %s\n", rdStr);

    ret = remove(path);
    if (ret != 0) {
        PRT_Printf("[LITTLEFS][ERROR] remove file failed, ret %d\n", ret);
        return -1;
    }

    return 0;
}

int LittlefsTest(void)
{
    int ret;

    PRT_Printf("[LITTLEFS][INFO] start sd3403 memory littlefs test\n");
    LittlefsRamMtdInit(&g_littlefsMtdDev, g_littlefsRamFlash, "ramlittlefs0");
    ret = register_mtddriver(LITTLEFS_DEV_NAME, &g_littlefsMtdDev.mtd, 0, &g_littlefsMtdDev);
    if (ret != 0) {
        PRT_Printf("[LITTLEFS][ERROR] register mtd driver failed, ret %d\n", ret);
        return -1;
    }

    ret = mount(LITTLEFS_DEV_NAME, LITTLEFS_MOUNT_POINT, "littlefs", 0, NULL);
    if (ret != 0) {
        PRT_Printf("[LITTLEFS][ERROR] mount failed, ret %d\n", ret);
        return -1;
    }

    ret = LittlefsFdOps(LITTLEFS_MOUNT_POINT "/test.txt");
    if (ret != 0) {
        PRT_Printf("[LITTLEFS][ERROR] file ops failed, ret %d\n", ret);
        return -1;
    }
    PRT_Printf("[LITTLEFS][INFO] sd3403 memory littlefs test success\n");
    return 0;
}
#else
int LittlefsTest(void)
{
    PRT_Printf("[LITTLEFS][INFO] skip, required macros are not enabled\n");
    return 0;
}
#endif

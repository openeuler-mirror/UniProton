#include "filesystem_test.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include "nuttx/fs/fs.h"
#include "nuttx/mtd/mtd.h"
#include "securec.h"

#define SPIFFS_DEV_NAME "/dev/ramspiffs0"
#define SPIFFS_MOUNT_POINT "/spiffs"
#define FLASH_BLOCK_SIZE 256
#define FLASH_ERASE_SIZE 4096
#define FLASH_ERASE_BLOCKS 128

extern unsigned int PRT_Printf(const char *format, ...);

#ifdef FS_ENABLE_SPIFFS_TEST
struct SpiffsRamMtdDev {
    struct mtd_dev_s mtd;
    unsigned char *data;
    size_t size;
    size_t blockSize;
    size_t eraseSize;
    size_t eraseBlocks;
};

static unsigned char g_spiffsRamFlash[FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS];
static struct SpiffsRamMtdDev g_spiffsMtdDev;

static ssize_t SpiffsRamMtdRead(struct mtd_dev_s *mtd, off_t offset, size_t nbytes, unsigned char *buffer)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;

    if (dev == NULL || buffer == NULL || offset < 0 || (size_t)offset + nbytes > dev->size) {
        return -EINVAL;
    }

    (void)memcpy_s(buffer, nbytes, dev->data + offset, nbytes);
    return (ssize_t)nbytes;
}

static ssize_t SpiffsRamMtdWrite(struct mtd_dev_s *mtd, off_t offset, size_t nbytes, const unsigned char *buffer)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;
    size_t i;

    if (dev == NULL || buffer == NULL || offset < 0 || (size_t)offset + nbytes > dev->size) {
        return -EINVAL;
    }

    for (i = 0; i < nbytes; i++) {
        dev->data[offset + i] &= buffer[i];
    }
    return (ssize_t)nbytes;
}

static int SpiffsRamMtdErase(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;

    if (dev == NULL || startblock < 0 || (size_t)startblock + nblocks > dev->eraseBlocks) {
        return -EINVAL;
    }

    (void)memset_s(dev->data + ((size_t)startblock * dev->eraseSize), nblocks * dev->eraseSize,
                   0xff, nblocks * dev->eraseSize);
    return (int)nblocks;
}

static ssize_t SpiffsRamMtdBread(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks, unsigned char *buffer)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;

    if (dev == NULL) {
        return -EINVAL;
    }
    return SpiffsRamMtdRead(mtd, startblock * dev->blockSize, nblocks * dev->blockSize, buffer) < 0 ? -EINVAL :
           (ssize_t)nblocks;
}

static ssize_t SpiffsRamMtdBwrite(struct mtd_dev_s *mtd, off_t startblock, size_t nblocks,
                                  const unsigned char *buffer)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;

    if (dev == NULL) {
        return -EINVAL;
    }
    return SpiffsRamMtdWrite(mtd, startblock * dev->blockSize, nblocks * dev->blockSize, buffer) < 0 ? -EINVAL :
           (ssize_t)nblocks;
}

static int SpiffsRamMtdIoctl(struct mtd_dev_s *mtd, int cmd, unsigned long arg)
{
    struct SpiffsRamMtdDev *dev = (struct SpiffsRamMtdDev *)mtd;

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

static void SpiffsRamMtdInit(struct SpiffsRamMtdDev *dev, unsigned char *data, const char *name)
{
    (void)memset_s(dev, sizeof(*dev), 0, sizeof(*dev));
    (void)memset_s(data, FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS, 0xff, FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS);
    dev->mtd.erase = SpiffsRamMtdErase;
    dev->mtd.bread = SpiffsRamMtdBread;
    dev->mtd.bwrite = SpiffsRamMtdBwrite;
    dev->mtd.read = SpiffsRamMtdRead;
    dev->mtd.write = SpiffsRamMtdWrite;
    dev->mtd.ioctl = SpiffsRamMtdIoctl;
    dev->mtd.name = name;
    dev->data = data;
    dev->size = FLASH_ERASE_SIZE * FLASH_ERASE_BLOCKS;
    dev->blockSize = FLASH_BLOCK_SIZE;
    dev->eraseSize = FLASH_ERASE_SIZE;
    dev->eraseBlocks = FLASH_ERASE_BLOCKS;
}

static int SpiffsFileOps(const char *path)
{
    FILE *fd = NULL;
    char wrStr[] = "hello sd3403 SPIFFS!";
    char rdStr[sizeof(wrStr)] = {0};
    size_t size;
    int ret;

    fd = fopen(path, "r");
    if (fd != NULL) {
        PRT_Printf("[SPIFFS][ERROR] open non-existent file with r success\n");
        (void)fclose(fd);
        return -1;
    }

    fd = fopen(path, "w");
    if (fd == NULL) {
        PRT_Printf("[SPIFFS][ERROR] open file for write failed\n");
        return -1;
    }

    size = fwrite(wrStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[SPIFFS][ERROR] write file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    fd = fopen(path, "r");
    if (fd == NULL) {
        PRT_Printf("[SPIFFS][ERROR] open file for read failed\n");
        return -1;
    }

    size = fread(rdStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[SPIFFS][ERROR] read file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    if (strcmp(wrStr, rdStr) != 0) {
        PRT_Printf("[SPIFFS][ERROR] read data mismatch: %s\n", rdStr);
        return -1;
    }

    PRT_Printf("[SPIFFS][INFO] read from file: %s\n", rdStr);

    ret = remove(path);
    if (ret != 0) {
        PRT_Printf("[SPIFFS][ERROR] remove file failed, ret %d\n", ret);
        return -1;
    }

    return 0;
}

int SpiffsTest(void)
{
    int ret;

    PRT_Printf("[SPIFFS][INFO] start sd3403 memory spiffs test\n");
    SpiffsRamMtdInit(&g_spiffsMtdDev, g_spiffsRamFlash, "ramspiffs0");
    ret = register_mtddriver(SPIFFS_DEV_NAME, &g_spiffsMtdDev.mtd, 0, &g_spiffsMtdDev);
    if (ret != 0) {
        PRT_Printf("[SPIFFS][ERROR] register mtd driver failed, ret %d\n", ret);
        return -1;
    }

    ret = mount(SPIFFS_DEV_NAME, SPIFFS_MOUNT_POINT, "spiffs", 0, NULL);
    if (ret != 0) {
        PRT_Printf("[SPIFFS][ERROR] mount failed, ret %d\n", ret);
        return -1;
    }

    ret = SpiffsFileOps(SPIFFS_MOUNT_POINT "/test.txt");
    if (ret != 0) {
        PRT_Printf("[SPIFFS][ERROR] file ops failed, ret %d\n", ret);
        return -1;
    }
    PRT_Printf("[SPIFFS][INFO] sd3403 memory spiffs test success\n");
    return 0;
}
#else
int SpiffsTest(void)
{
    PRT_Printf("[SPIFFS][INFO] skip, required macros are not enabled\n");
    return 0;
}
#endif

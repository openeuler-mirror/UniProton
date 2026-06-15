#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include "nuttx/fs/fs.h"
#include "nuttx/fs/ioctl.h"
#include "securec.h"

#define FATFS_DEV_NAME "/dev/ramfat0"
#define FATFS_SECTOR_SIZE 512
#define FATFS_SECTOR_COUNT 2048

struct FatfsRamDev {
    unsigned char *data;
    size_t sectors;
    size_t sectorSize;
};

static unsigned char g_fatfsRamDisk[FATFS_SECTOR_SIZE * FATFS_SECTOR_COUNT];
static struct FatfsRamDev g_fatfsRamDev = {g_fatfsRamDisk, FATFS_SECTOR_COUNT, FATFS_SECTOR_SIZE};

extern unsigned int PRT_Printf(const char *format, ...);
extern void fs_initialize(void);

static ssize_t FatfsRamRead(struct inode *inode, unsigned char *buffer, blkcnt_t startSector, unsigned int nsectors)
{
    struct FatfsRamDev *dev = (struct FatfsRamDev *)inode->i_private;

    if (dev == NULL || buffer == NULL || startSector < 0 || (size_t)startSector + nsectors > dev->sectors) {
        PRT_Printf("[FATFS][ERROR] read invalid, sector %d count %u\n", (int)startSector, nsectors);
        return -EINVAL;
    }

    (void)memcpy_s(buffer, nsectors * dev->sectorSize,
                   dev->data + ((size_t)startSector * dev->sectorSize), nsectors * dev->sectorSize);
    return (ssize_t)nsectors;
}

static ssize_t FatfsRamWrite(struct inode *inode, const unsigned char *buffer, blkcnt_t startSector,
                             unsigned int nsectors)
{
    struct FatfsRamDev *dev = (struct FatfsRamDev *)inode->i_private;

    if (dev == NULL || buffer == NULL || startSector < 0 || (size_t)startSector + nsectors > dev->sectors) {
        PRT_Printf("[FATFS][ERROR] write invalid, sector %d count %u\n", (int)startSector, nsectors);
        return -EINVAL;
    }

    (void)memcpy_s(dev->data + ((size_t)startSector * dev->sectorSize), nsectors * dev->sectorSize,
                   buffer, nsectors * dev->sectorSize);
    return (ssize_t)nsectors;
}

static int FatfsRamGeometry(struct inode *inode, struct geometry *geometry)
{
    struct FatfsRamDev *dev = (struct FatfsRamDev *)inode->i_private;

    if (dev == NULL || geometry == NULL) {
        return -EINVAL;
    }

    (void)memset_s(geometry, sizeof(*geometry), 0, sizeof(*geometry));
    geometry->geo_available = true;
    geometry->geo_writeenabled = true;
    geometry->geo_nsectors = dev->sectors;
    geometry->geo_sectorsize = dev->sectorSize;
    return 0;
}

static int FatfsRamIoctl(struct inode *inode, int cmd, unsigned long arg)
{
    struct FatfsRamDev *dev = (struct FatfsRamDev *)inode->i_private;

    if (dev == NULL) {
        return -EINVAL;
    }

    switch (cmd) {
        case BIOC_PARTINFO:
        {
            struct partition_info_s *info = (struct partition_info_s *)arg;
            if (info == NULL) {
                return -EINVAL;
            }
            (void)memset_s(info, sizeof(*info), 0, sizeof(*info));
            info->numsectors = dev->sectors;
            info->sectorsize = dev->sectorSize;
            return 0;
        }
        case BIOC_FLUSH:
            return 0;
        default:
            return -ENOTTY;
    }
}

static const struct block_operations g_fatfsRamBops = {
    NULL,
    NULL,
    FatfsRamRead,
    FatfsRamWrite,
    FatfsRamGeometry,
    FatfsRamIoctl,
    NULL,
};

static int FatfsFileOps(void)
{
    FILE *fd = NULL;
    char wrStr[] = "hello sd3403 fatfs!";
    char rdStr[sizeof(wrStr)] = {0};
    size_t size;
    int ret;

    fd = fopen("/test.txt", "r");
    if (fd != NULL) {
        PRT_Printf("[FATFS][ERROR] open non-existent file with r success\n");
        (void)fclose(fd);
        return -1;
    }

    fd = fopen("/test.txt", "w");
    if (fd == NULL) {
        PRT_Printf("[FATFS][ERROR] open file for write failed\n");
        return -1;
    }

    size = fwrite(wrStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[FATFS][ERROR] write file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    fd = fopen("/test.txt", "r");
    if (fd == NULL) {
        PRT_Printf("[FATFS][ERROR] open file for read failed\n");
        return -1;
    }

    size = fread(rdStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[FATFS][ERROR] read file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    if (strcmp(wrStr, rdStr) != 0) {
        PRT_Printf("[FATFS][ERROR] read data mismatch: %s\n", rdStr);
        return -1;
    }

    PRT_Printf("[FATFS][INFO] read from file: %s\n", rdStr);

    ret = remove("/test.txt");
    if (ret != 0) {
        PRT_Printf("[FATFS][ERROR] remove file failed, ret %d\n", ret);
        return -1;
    }

    return 0;
}

void fatfs_test(void)
{
    int ret;

    PRT_Printf("[FATFS][INFO] start sd3403 memory fatfs test\n");
    fs_initialize();

    (void)memset_s(g_fatfsRamDisk, sizeof(g_fatfsRamDisk), 0, sizeof(g_fatfsRamDisk));
    ret = register_blockdriver(FATFS_DEV_NAME, &g_fatfsRamBops, 0, &g_fatfsRamDev);
    if (ret != 0) {
        PRT_Printf("[FATFS][ERROR] register block driver failed, ret %d\n", ret);
        return;
    }
    PRT_Printf("[FATFS][INFO] register block driver success\n");

    ret = mount(FATFS_DEV_NAME, "/", "vfat", 0, "0:");
    if (ret != 0) {
        PRT_Printf("[FATFS][ERROR] mount failed, ret %d\n", ret);
        return;
    }
    PRT_Printf("[FATFS][INFO] mount success\n");

    ret = FatfsFileOps();
    if (ret != 0) {
        PRT_Printf("[FATFS][ERROR] file ops failed, ret %d\n", ret);
        return;
    }

    PRT_Printf("[FATFS][INFO] sd3403 memory fatfs test success\n");
}

#include "filesystem_test.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "nuttx/fs/fs.h"
#include "securec.h"

#define DEVFS_DEV_NAME "/dev/ramdevfs0"

extern unsigned int PRT_Printf(const char *format, ...);

#ifdef FS_ENABLE_DEVFS_TEST
struct DevfsRamDev {
    char data[64];
    size_t size;
};

static struct DevfsRamDev g_devfsRamDev;

static int DevfsOpen(struct file *filep)
{
    filep->f_priv = filep->f_inode->i_private;
    return 0;
}

static ssize_t DevfsRead(struct file *filep, char *buffer, size_t buflen)
{
    struct DevfsRamDev *dev = (struct DevfsRamDev *)filep->f_priv;
    size_t left;

    if (dev == NULL || buffer == NULL) {
        return -EINVAL;
    }
    if ((size_t)filep->f_pos >= dev->size) {
        return 0;
    }
    left = dev->size - (size_t)filep->f_pos;
    if (buflen > left) {
        buflen = left;
    }
    (void)memcpy_s(buffer, buflen, dev->data + filep->f_pos, buflen);
    filep->f_pos += buflen;
    return (ssize_t)buflen;
}

static ssize_t DevfsWrite(struct file *filep, const char *buffer, size_t buflen)
{
    struct DevfsRamDev *dev = (struct DevfsRamDev *)filep->f_priv;

    if (dev == NULL || buffer == NULL) {
        return -EINVAL;
    }
    if (buflen > sizeof(dev->data)) {
        buflen = sizeof(dev->data);
    }
    (void)memcpy_s(dev->data, sizeof(dev->data), buffer, buflen);
    dev->size = buflen;
    filep->f_pos = buflen;
    return (ssize_t)buflen;
}

static off_t DevfsSeek(struct file *filep, off_t offset, int whence)
{
    struct DevfsRamDev *dev = (struct DevfsRamDev *)filep->f_priv;
    off_t pos;

    if (dev == NULL) {
        return -EINVAL;
    }
    if (whence == SEEK_SET) {
        pos = offset;
    } else if (whence == SEEK_CUR) {
        pos = filep->f_pos + offset;
    } else if (whence == SEEK_END) {
        pos = (off_t)dev->size + offset;
    } else {
        return -EINVAL;
    }
    if (pos < 0) {
        pos = 0;
    }
    if ((size_t)pos > dev->size) {
        pos = (off_t)dev->size;
    }
    filep->f_pos = pos;
    return pos;
}

static const struct file_operations g_devfsFops = {
    DevfsOpen,
    NULL,
    DevfsRead,
    DevfsWrite,
    DevfsSeek,
    NULL,
    NULL,
    NULL,
    NULL,
};

int DevfsTest(void)
{
    int ret;
    int fd;
    char wrStr[] = "hello sd3403 devfs!";
    char rdStr[sizeof(wrStr)] = {0};

    PRT_Printf("[DEVFS][INFO] start sd3403 devfs test\n");
    (void)memset_s(&g_devfsRamDev, sizeof(g_devfsRamDev), 0, sizeof(g_devfsRamDev));
    ret = register_driver(DEVFS_DEV_NAME, &g_devfsFops, 0, &g_devfsRamDev);
    if (ret != 0) {
        PRT_Printf("[DEVFS][ERROR] register driver failed, ret %d\n", ret);
        return -1;
    }

    fd = open(DEVFS_DEV_NAME, O_RDWR);
    if (fd < 0) {
        PRT_Printf("[DEVFS][ERROR] open device failed\n");
        return -1;
    }
    if (write(fd, wrStr, sizeof(wrStr)) != sizeof(wrStr)) {
        PRT_Printf("[DEVFS][ERROR] write device failed\n");
        (void)close(fd);
        return -1;
    }
    (void)lseek(fd, 0, SEEK_SET);
    if (read(fd, rdStr, sizeof(rdStr)) != sizeof(rdStr)) {
        PRT_Printf("[DEVFS][ERROR] read device failed\n");
        (void)close(fd);
        return -1;
    }
    (void)close(fd);
    if (strcmp(wrStr, rdStr) != 0) {
        PRT_Printf("[DEVFS][ERROR] read data mismatch: %s\n", rdStr);
        return -1;
    }
    (void)unregister_driver(DEVFS_DEV_NAME);
    PRT_Printf("[DEVFS][INFO] sd3403 devfs test success\n");
    return 0;
}
#else
int DevfsTest(void)
{
    PRT_Printf("[DEVFS][INFO] skip, required macros are not enabled\n");
    return 0;
}
#endif

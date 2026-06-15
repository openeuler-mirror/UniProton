#include "filesystem_test.h"
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include "securec.h"

#define RAMFS_MOUNT_POINT "/ram"
#define RAMFS_CAPACITY (128 * 1024)

extern unsigned int PRT_Printf(const char *format, ...);

#ifdef FS_ENABLE_RAMFS_TEST
static int RamfsFileOps(const char *path)
{
    FILE *fd = NULL;
    char wrStr[] = "hello sd3403 RAMFS!";
    char rdStr[sizeof(wrStr)] = {0};
    size_t size;
    int ret;

    fd = fopen(path, "r");
    if (fd != NULL) {
        PRT_Printf("[RAMFS][ERROR] open non-existent file with r success\n");
        (void)fclose(fd);
        return -1;
    }

    fd = fopen(path, "w");
    if (fd == NULL) {
        PRT_Printf("[RAMFS][ERROR] open file for write failed\n");
        return -1;
    }

    size = fwrite(wrStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[RAMFS][ERROR] write file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    fd = fopen(path, "r");
    if (fd == NULL) {
        PRT_Printf("[RAMFS][ERROR] open file for read failed\n");
        return -1;
    }

    size = fread(rdStr, sizeof(char), sizeof(wrStr), fd);
    if (size != sizeof(wrStr)) {
        PRT_Printf("[RAMFS][ERROR] read file failed, size %u\n", (unsigned int)size);
        (void)fclose(fd);
        return -1;
    }
    (void)fclose(fd);

    if (strcmp(wrStr, rdStr) != 0) {
        PRT_Printf("[RAMFS][ERROR] read data mismatch: %s\n", rdStr);
        return -1;
    }

    PRT_Printf("[RAMFS][INFO] read from file: %s\n", rdStr);

    ret = remove(path);
    if (ret != 0) {
        PRT_Printf("[RAMFS][ERROR] remove file failed, ret %d\n", ret);
        return -1;
    }

    return 0;
}

int RamfsTest(void)
{
    int ret;

    PRT_Printf("[RAMFS][INFO] start sd3403 ramfs test\n");
    ret = mount(NULL, RAMFS_MOUNT_POINT, "ramfs", 0, (const void *)RAMFS_CAPACITY);
    if (ret != 0) {
        PRT_Printf("[RAMFS][ERROR] mount failed, ret %d\n", ret);
        return -1;
    }

    ret = RamfsFileOps(RAMFS_MOUNT_POINT "/test.txt");
    if (ret != 0) {
        PRT_Printf("[RAMFS][ERROR] file ops failed, ret %d\n", ret);
        return -1;
    }
    PRT_Printf("[RAMFS][INFO] sd3403 ramfs test success\n");
    return 0;
}
#else
int RamfsTest(void)
{
    PRT_Printf("[RAMFS][INFO] skip, required macros are not enabled\n");
    return 0;
}
#endif

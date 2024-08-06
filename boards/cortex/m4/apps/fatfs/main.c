#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_idle.h"
#include "test.h"
#if defined(_SIM_)
#include "semihosting_dbg.h"
#else
#include "rtt_viewer.h"
#endif
#include "nuttx/mtd/mtd.h"

#define FLASH_OFFSET 256
#define FLASH_SIZE OS_OPTION_FLASH_SIZE
#define KB_PRE_BYTE 1024

char *flash_dev_name = "/dev/flash0";

static int file_ops()
{
    FILE *fd = NULL;
    char wr_str[] = "hello fs!";
    char rd_str[sizeof(wr_str)];
    fd = fopen("/test.txt", "r");
    if (fd != NULL) {
        printf("[ERROR] open not exists file with 'r' success !\n");
        return -1;
    }
    fd = fopen("/test.txt", "w");
    if (fd == NULL) {
        printf("[ERROR] open not exists file with 'w' fail !\n");
        return -1;
    }
    int ret = fwrite(wr_str, sizeof(char), sizeof(wr_str), fd);
    if (ret <= 0) {
        printf("[ERROR] write file fail !\n");
        return -1;
    }
    fclose(fd);
    fd = NULL;
    fd = fopen("/test.txt", "r");
    if (fd == NULL) {
        printf("[ERROR] open exists file with 'r' fail !\n");
        return -1;
    }
    ret = fread(rd_str, sizeof(char), sizeof(wr_str), fd);
    if (ret <= 0) {
        printf("[ERROR] read file fail !\n");
        return -1;
    }
    if (strcmp(wr_str, rd_str) != 0) {
        printf("[ERROR] read file info not right !\n");
        return -1;
    }
    printf("[INFO]read from file is : %s\n", rd_str);
    fclose(fd);

    ret = remove("/test.txt");
    if (ret != 0) {
        printf("[ERROR] remove file fail !\n");
        return -1;
    }

    return 0;
}

void fatfs_task(U32 uwParam1, U32 uParam2, U32 uwParam3, U32 uwParam4)
{
    struct mtd_dev_s *mtd;
    struct mtd_dev_s *mtd_part;

    int offset;
    int size;
    int ret;

    fs_initialize();
    stm32_clockconfig();
    stm32_lowsetup();

    mtd = progmem_initialize();
    if (mtd == NULL) {
        printf("[ERROR] Fail to get flash mtd\n");
        return;
    }
    printf("[INFO] initialize successed !\n");

    offset = up_progmem_getpage((FLASH_OFFSET * KB_PRE_BYTE) + up_progmem_getaddress(0));
    size = (FLASH_SIZE * KB_PRE_BYTE) / up_progmem_pagesize(offset);
    mtd_part = mtd_partition(mtd, offset, size);
    if (mtd_part == NULL) {
        printf("[ERROR] Fail to partition flash mtd\n");
        return;
    }
    printf("[INFO] mtd_partition successed !\n");

    ret = ftl_initialize_by_path(flash_dev_name, mtd_part);
    if (ret != 0) {
        printf("[ERROR] Fail to initialize flash mtd as block driver, ret %d\n", ret);
        return;
    }
    printf("[INFO] ftl_initialize_by_path successed !\n");

    ret = mount(flash_dev_name, "/", "vfat", 0, flash_dev_name);
    if (ret != 0) {
        printf("Fail to mount fd, ret %d\n", ret);
        return;
    }
    printf("[INFO] mount successed !\n");

    ret = file_ops();
    if (ret != 0) {
        printf("Fail to run file ops, ret %d\n", ret);
        return;
    }
    printf("[INFO] file_ops successed !\n");

    printf("[INFO]End fatfs test");

    return;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskPid;
    struct TskInitParam stInitParam = {fatfs_task, 10, 0, {0}, 0x800, "TaskA", 0};

    ret = PRT_TaskCreate(&taskPid, &stInitParam);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(taskPid);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
#if !defined(_SIM_)
    RttViewerInit();
    RttViewerModeSet(0, RTT_VIEWER_MODE_BLOCK_IF_FIFO_FULL);
#endif

    return OS_OK;
}

extern U32 __data_start__;
extern U32 __data_end__;
extern U32 __text_end__;
void OsGlobalDataInit(void)
{
    U32 size;
    U32 *dest = (U32 *)&__data_start__;
    U32 *src = (U32 *)&__text_end__;
    U32 i;

    size = (U32)&__data_end__ - (U32)&__data_start__;
    for (i = 0; i < (size / 4); i++) {
        dest[i] = src[i];
    }
}

void PRT_HardBootInit(void)
{
    OsGlobalDataInit();
}

U32 PRT_Printf(const char *format, ...)
{
    va_list vaList;
    char buff[0x200] = { 0 };
    S32 count;
    U32 ret;

    va_start(vaList, format);
    count = vsprintf_s(buff, 0x200, format, vaList);
    va_end(vaList);

    if (count == -1) {
        return OS_ERROR;
    }

#if defined(_SIM_)
    SemihostingDbgWrite(buff, count);
#else
    RttViewerWrite(0, buff, count);
#endif

    return count;
}

S32 main(void)
{
    return OsConfigStart();
}

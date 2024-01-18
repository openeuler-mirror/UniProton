/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stddef.h>
#include <sys/time.h>
#include "ff.h"            /* Obtains integer types */
#include "diskio.h"        /* Declarations of disk functions */
#include "nuttx/fs/fs.h"
#include "nuttx/fs/ioctl.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM        0    /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC        1    /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB        2    /* Example: Map USB MSD to physical drive 2 */

extern struct inode * regist_inode[FF_VOLUMES];

static DSTATUS check_regist_inode (
    BYTE pdrv        /* Physical drive nmuber to identify the drive */
)
{
    if (pdrv >= FF_VOLUMES) {
        return STA_NODISK;
    }
    if (regist_inode[pdrv] == NULL) {
        return STA_NODISK;
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive nmuber to identify the drive */
)
{
    return check_regist_inode(pdrv);
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv                /* Physical drive nmuber to identify the drive */
)
{
    return check_regist_inode(pdrv);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    BYTE *buff,        /* Data buffer to store read data */
    LBA_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
)
{
    DSTATUS status = 0;
    status = check_regist_inode(pdrv);
    if (status != RES_OK) {
        return status;
    }

    struct inode *inode = regist_inode[pdrv];
    if (inode->u.i_bops == NULL || inode->u.i_bops->read == NULL) {
        return RES_NOTRDY;
    }

    ssize_t read_count =  inode->u.i_bops->read(inode, buff, sector, count);
    if (read_count == count) {
        return RES_OK;
    }

    return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,            /* Physical drive nmuber to identify the drive */
    const BYTE *buff,    /* Data to be written */
    LBA_t sector,        /* Start sector in LBA */
    UINT count            /* Number of sectors to write */
)
{
    DSTATUS status = 0;
    status = check_regist_inode(pdrv);
    if (status != RES_OK) {
        return status;
    }

    struct inode *inode = regist_inode[pdrv];
    if (inode->u.i_bops == NULL || inode->u.i_bops->write == NULL) {
        return RES_NOTRDY;
    }

    ssize_t write_count =  inode->u.i_bops->write(inode, buff, sector, count);
    if (write_count == count) {
        return RES_OK;
    }

    return RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    DSTATUS status = 0;
    status = check_regist_inode(pdrv);
    if (status != RES_OK) {
        return status;
    }

    struct inode *inode = regist_inode[pdrv];
    if (inode->u.i_bops == NULL || inode->u.i_bops->ioctl == NULL) {
        return RES_NOTRDY;
    }

    struct partition_info_s info = { 0 };
    int ret =  inode->u.i_bops->ioctl(inode, BIOC_PARTINFO, (unsigned long)&info);
    if (ret != RES_OK) {
        return RES_ERROR;
    }
    switch (cmd) {
        case CTRL_SYNC:
            inode->u.i_bops->ioctl(inode, BIOC_FLUSH, (unsigned long)&info);
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = info.numsectors;
            break;
        case GET_SECTOR_SIZE:
            *(DWORD *)buff = info.sectorsize;
            break;
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1;
            break;
        case CTRL_TRIM:
        default:
            break;
    }
    ret = RES_OK;

    return ret;
}

DWORD get_fattime (void)
{
    struct timeval tv;
    int ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        return 0;
    }
    return (DWORD)tv.tv_sec;
}


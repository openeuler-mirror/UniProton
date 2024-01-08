/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-18
 * Description: 修改oflags的测试用例
 */

#include <stdio.h>
#include "posixtest.h"

// 根据nuttx原本的宏定义
#define O_RDONLY_OLD  01
#define O_WRONLY_OLD  02
#define O_RDWR_OLD    03

// 根据musl的宏定义
#define O_RDONLY_NEW  00
#define O_WRONLY_NEW  01
#define O_RDWR_NEW    02

#define OTHER 0100000
#define MASK 03
#define ERR_1 00
#define ERR_2 03

static int testcases[] = {
    ERR_1, ERR_2,   //异常情况
    O_RDONLY_OLD, O_RDONLY_NEW,
    O_WRONLY_OLD, O_WRONLY_NEW,
    O_RDWR_OLD, O_RDWR_NEW,
    O_RDONLY_OLD | O_WRONLY_OLD, O_RDWR_NEW,
    OTHER | O_RDONLY_OLD, OTHER | O_RDONLY_NEW,
    OTHER | O_WRONLY_OLD, OTHER | O_WRONLY_NEW,
    OTHER | O_RDWR_OLD, OTHER | O_RDWR_NEW,
    OTHER | O_RDONLY_OLD | O_WRONLY_OLD, OTHER | O_RDWR_NEW,
};

/**
 * 修改mount_open函数(fs_procfs_mount.c:377)
*/
int TEST_rwflag_1(void)
{
    int oflag1, oflag2;
    int ok_1, ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];
        ok_1 = ((oflag1 & O_WRONLY_OLD) != 0 || (oflag1 & O_RDONLY_OLD) == 0);
        ok_2 = ((oflag2 & MASK) != O_RDONLY_NEW);

        if (ok_1 != ok_2) {
            printf("TEST_rwflag_1 error! i=%d, oflag1 = %x, oflag2=%x, ok_1 = %d, ok_2 = %d\n", i, oflag1, oflag2, ok_1, ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}

/**
 * 修改timerfd_create函数(fs_timerfd.c:461)，
 * 以及signalfd函数(fs_signalfd.c:351)
*/
int TEST_rwflag_2(void)
{
    int oflag1, oflag2;
    int read_ok_1, read_ok_2, write_ok_1, write_ok_2, rw_ok_1, rw_ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        if (i == 0) continue;
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];

        oflag1 = (O_RDONLY_OLD | oflag1);
        read_ok_1 = ((oflag1 & O_RDONLY_OLD) != 0);    //测试可读性
        write_ok_1 = ((oflag1 & O_WRONLY_OLD) != 0);   //测试可写性

        oflag2 = ((oflag2 & MASK) == O_WRONLY_NEW) ? (oflag2 + 1) : oflag2;
        read_ok_2 = (((oflag2 & MASK) == O_RDONLY_NEW) || ((oflag2 & MASK) == O_RDWR_NEW));
        write_ok_2 = (((oflag2 & MASK) == O_WRONLY_NEW) || ((oflag2 & MASK) == O_RDWR_NEW));

        if (read_ok_1 != read_ok_2 || write_ok_1 != write_ok_2 || (oflag1 >> 2) != (oflag2 >> 2)) {
            printf("TEST_rwflag_2 error! i=%d, oflag1=%x, oflag2=%x, read_ok_1=%d, read_ok_2=%d, write_ok_1=%d, write_ok_2=%d\n", 
                i, oflag1, oflag2, read_ok_1, read_ok_2, write_ok_1, write_ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}

/**
 * 修改block_proxy函数(fs_blockproxy.c:163)，
 * 及修改file_truncate函数(fs_truncate.c.c:57)，
 * 及修改file_write函数(fs_write.c:74)
*/
int TEST_rwflag_3(void)
{
    int oflag1, oflag2;
    int ok_1, ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];
    
        ok_1 = ((oflag1 & O_WRONLY_OLD) == 0);
        ok_2 = ((oflag2 & MASK) != O_WRONLY_NEW && (oflag2 & MASK) != O_RDWR_NEW);

        if (ok_1 != ok_2) {
            printf("TEST_rwflag_3 error! i=%d, oflag1 = %x, oflag2=%x, ok_1 = %d, ok_2 = %d\n", i, oflag1, oflag2, ok_1, ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}

/**
 * 修改file_read函数(fs_read.c.c:74)
*/
int TEST_rwflag_4(void)
{
    int oflag1, oflag2;
    int ok_1, ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];
    
        ok_1 = ((oflag1 & O_RDONLY_OLD) == 0);
        ok_2 = ((oflag2 & MASK) != O_RDONLY_NEW && (oflag2 & MASK) != O_RDWR_NEW);

        if (ok_1 != ok_2) {
            printf("TEST_rwflag_4 error! i=%d, oflag1 = %x, oflag2=%x, ok_1 = %d, ok_2 = %d\n", i, oflag1, oflag2, ok_1, ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}

/**
 * 修改inode_checkflags函数(fs_open.c.c:302)
 * 修改pipecommon_open函数(pipe_common.c:170)
 * 修改pipecommon_open函数(pipe_common.c:237)
*/
int TEST_rwflag_5(void)
{
    int oflag1, oflag2;
    int read_ok_1, read_ok_2, write_ok_1, write_ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];
    
        read_ok_1 = ((oflag1 & O_RDONLY_OLD) != 0);
        write_ok_1 = ((oflag1 & O_WRONLY_OLD) != 0);
        
        read_ok_2 = ((oflag2 & MASK) == O_RDONLY_NEW || (oflag2 & MASK) == O_RDWR_NEW);
        write_ok_2 = ((oflag2 & MASK) == O_WRONLY_NEW || (oflag2 & MASK) == O_RDWR_NEW);

        if (read_ok_1 != read_ok_2 || write_ok_1 != write_ok_2) {
            printf("TEST_rwflag_5 error! i=%d, oflag1 = %x, oflag2=%x, ok_1 = %d, ok_2 = %d\n", i, oflag1, oflag2, read_ok_1, read_ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}

/**
 * 修改pipecommon_open函数(pipe_common.c:189)
*/
int TEST_rwflag_6(void)
{
    int oflag1, oflag2;
    int ok_1, ok_2;
    int len = sizeof(testcases) / sizeof(int);
    for (int i = 0; i < len; i += 2) {
        oflag1 = testcases[i];
        oflag2 = testcases[i+1];
    
        ok_1 = (oflag1 & O_RDWR_OLD) == O_WRONLY_OLD;
        ok_2 = (oflag2 & MASK) == O_WRONLY_NEW;

        if (ok_1 != ok_2) {
            printf("TEST_rwflag_6 error! i=%d, oflag1 = %x, oflag2=%x, ok_1 = %d, ok_2 = %d\n", i, oflag1, oflag2, ok_1, ok_2);
            return PTS_FAIL;
        }
    }

    return PTS_PASS;
}
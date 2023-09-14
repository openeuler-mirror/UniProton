/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-30
 * Description: exit测试定义
 */
#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int _Exit_1_1();
extern int _Exit_1_2();
extern int abort_1_1();
extern int abort_1_2();
extern int at_quick_exit_1_1();
extern int at_quick_exit_1_2();
extern int at_quick_exit_1_3();
extern int atexit_1_1();
extern int atexit_1_2();
extern int atexit_1_3();
extern int exit_1_1();
extern int exit_1_2();
extern int quick_exit_1_1();
extern int quick_exit_1_2();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
#ifdef _EXIT_1_1
    _Exit_1_1,
#endif
#ifdef _EXIT_1_2
    _Exit_1_2,
#endif
#ifdef ABORT_1_1
    abort_1_1,
#endif
#ifdef ABORT_1_2
    abort_1_2,
#endif
#ifdef AT_QUICK_EXIT_1_1
    at_quick_exit_1_1,
#endif
#ifdef AT_QUICK_EXIT_1_2
    at_quick_exit_1_2,
#endif
#ifdef AT_QUICK_EXIT_1_3
    at_quick_exit_1_3,
#endif
#ifdef ATEXIT_1_1
    atexit_1_1,
#endif
#ifdef ATEXIT_1_2
    atexit_1_2,
#endif
#ifdef ATEXIT_1_3
    atexit_1_3,
#endif
#ifdef EXIT_1_1
    exit_1_1,
#endif
#ifdef EXIT_1_2
    exit_1_2,
#endif
#ifdef QUICK_EXIT_1_1
    quick_exit_1_1,
#endif
#ifdef QUICK_EXIT_1_2
    quick_exit_1_2,
#endif
};

char run_test_name_1[][50] = {
#ifdef _EXIT_1_1
    "_Exit_1_1",
#endif
#ifdef _EXIT_1_2
    "_Exit_1_2",
#endif
#ifdef ABORT_1_1
    "abort_1_1",
#endif
#ifdef ABORT_1_2
    "abort_1_2",
#endif
#ifdef AT_QUICK_EXIT_1_1
    "at_quick_exit_1_1",
#endif
#ifdef AT_QUICK_EXIT_1_2
    "at_quick_exit_1_2",
#endif
#ifdef AT_QUICK_EXIT_1_3
    "at_quick_exit_1_3",
#endif
#ifdef ATEXIT_1_1
    "atexit_1_1",
#endif
#ifdef ATEXIT_1_2
    "atexit_1_2",
#endif
#ifdef ATEXIT_1_3
    "atexit_1_3",
#endif
#ifdef EXIT_1_1
    "exit_1_1",
#endif
#ifdef EXIT_1_2
    "exit_1_2",
#endif
#ifdef QUICK_EXIT_1_1
    "quick_exit_1_1",
#endif
#ifdef QUICK_EXIT_1_2
    "quick_exit_1_2",
#endif
};

#endif
#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int clock_gettime_7_1();
extern int clock_gettime_8_1();
extern int clock_gettime_8_2();
extern int clock_gettime_3_1();
extern int clock_gettime_4_1();
extern int clock_gettime_2_1();
extern int clock_gettime_1_1();
extern int clock_gettime_1_2();
extern int clock_settime_19_1();
extern int clock_settime_6_1();
extern int clock_settime_1_1();
extern int clock_settime_17_2();
extern int clock_settime_17_1();
extern int clock_settime_20_1();
extern int clock_getres_7_1();
extern int clock_getres_6_2();
extern int clock_getres_5_1();
extern int clock_getres_8_1();
extern int clock_getres_6_1();
extern int clock_getres_3_1();
extern int clock_getres_1_1();
extern int nanosleep_5_1();
extern int nanosleep_3_2();
extern int nanosleep_10000_1();
extern int nanosleep_6_1();
extern int nanosleep_2_1();
extern int nanosleep_1_1();
extern int nanosleep_1_2();
extern int timer_create_15_1();
extern int timer_create_16_1();
extern int timer_delete_5_1();
extern int timer_delete_1_2();
extern int timer_settime_8_3();
extern int timer_settime_12_3();
extern int timer_settime_12_1();
extern int timer_settime_12_2();
extern int timer_settime_8_1();
extern int timer_settime_8_2();
extern int timer_settime_13_1();
extern int timer_settime_8_4();
extern int timer_gettime_6_2();
extern int timer_gettime_6_1();
extern int timer_gettime_6_3();
extern int timer_gettime_2_2();
extern int timer_gettime_3_1();
extern int timer_gettime_2_1();
extern int timer_gettime_1_1();
extern int timer_gettime_1_2();
extern int timer_gettime_1_3();
extern int timer_gettime_1_4();

extern int asctime_1_1();
extern int clock_1_1();
extern int clock_2_1();
extern int clock_getcpuclockid_1_1();
extern int clock_getcpuclockid_2_1();

extern int clock_nanosleep_1_1();
extern int clock_nanosleep_2_1();
extern int clock_nanosleep_11_1();
extern int clock_nanosleep_13_1();

extern int ctime_1_1();
extern int difftime_1_1();
extern int gmtime_1_1();
extern int gmtime_2_1();
extern int localtime_1_1();
extern int mktime_1_1();

extern int strftime_3_1();
extern int timer_getoverrun_6_1();

extern int getpagesize_1_1();
extern int usleep_1_1();
extern int gettimeofday_1_1();
extern int time_1_1();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    clock_gettime_7_1,
    clock_gettime_8_1,
    clock_gettime_8_2,
    clock_gettime_3_1,
    clock_gettime_1_1,
    clock_settime_19_1,
    clock_settime_6_1,
    clock_settime_1_1,
    clock_settime_17_2,
    clock_settime_17_1,
    clock_settime_20_1,
    clock_getres_6_2,
    clock_getres_5_1,
    clock_getres_6_1,
    clock_getres_3_1,
    clock_getres_1_1,
    nanosleep_5_1,
    nanosleep_10000_1,
    nanosleep_6_1,
    nanosleep_2_1,
    nanosleep_1_1,
    timer_create_15_1,
    timer_create_16_1,
    timer_delete_5_1,
    timer_settime_12_1,
    timer_gettime_6_1,

    asctime_1_1,
    clock_1_1,
    clock_2_1,
    clock_getcpuclockid_1_1,

    clock_nanosleep_1_1,
    clock_nanosleep_2_1,
    clock_nanosleep_11_1,
    clock_nanosleep_13_1,

    ctime_1_1,
    difftime_1_1,
    gmtime_1_1,
    gmtime_2_1,

    localtime_1_1,
    mktime_1_1,

    strftime_3_1,
    timer_getoverrun_6_1,

    getpagesize_1_1,
    usleep_1_1,
    gettimeofday_1_1,
    time_1_1
};

char run_test_name_1[][50] = {
    "clock_gettime_7_1",
    "clock_gettime_8_1",
    "clock_gettime_8_2",
    "clock_gettime_3_1",
    "clock_gettime_1_1",
    "clock_settime_19_1",
    "clock_settime_6_1",
    "clock_settime_1_1",
    "clock_settime_17_2",
    "clock_settime_17_1",
    "clock_settime_20_1",
    "clock_getres_6_2",
    "clock_getres_5_1",
    "clock_getres_6_1",
    "clock_getres_3_1",
    "clock_getres_1_1",
    "nanosleep_5_1",
    "nanosleep_10000_1",
    "nanosleep_6_1",
    "nanosleep_2_1",
    "nanosleep_1_1",
    "timer_create_15_1",
    "timer_create_16_1",
    "timer_delete_5_1",
    "timer_settime_12_1",
    "timer_gettime_6_1",

    "asctime_1_1",
    "clock_1_1",
    "clock_2_1",
    "clock_getcpuclockid_1_1",

    "clock_nanosleep_1_1",
    "clock_nanosleep_2_1",
    "clock_nanosleep_11_1",
    "clock_nanosleep_13_1",

    "ctime_1_1",
    "difftime_1_1",
    "gmtime_1_1",
    "gmtime_2_1",

    "localtime_1_1",
    "mktime_1_1",

    "strftime_3_1",
    "timer_getoverrun_6_1",

    "getpagesize_1_1",
    "usleep_1_1",
    "gettimeofday_1_1",
    "time_1_1"
};



#endif
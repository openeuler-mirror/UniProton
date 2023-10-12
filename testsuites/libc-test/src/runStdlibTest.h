#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int qsort_test();  
extern int wcstol_test();  
extern int strtol_test(); 
extern int strtod_test(); 
extern int div_test();
extern int fcvt_test();
extern int llabs_test();
extern int atof_test();
extern int imaxabs_test();
extern int lldiv_test();
extern int ldiv_test();
extern int abs_test();
extern int wcstod_test();
extern int atoll_test();
extern int imaxdiv_test();
extern int atoi_test();
extern int atol_test();
extern int labs_test();
extern int ecvt_test();
extern int gcvt_test();
extern int bsearch_test();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    qsort_test,
    wcstol_test,
    strtol_test,
    strtod_test,
    div_test,
    fcvt_test,
    llabs_test,
    atof_test,
    imaxabs_test,
    lldiv_test,
    ldiv_test,
    abs_test,
    wcstod_test,
    atoll_test,
    imaxdiv_test,
    atoi_test,
    atol_test,
    labs_test,
    ecvt_test,
    gcvt_test,
    bsearch_test,
};

char run_test_name_1[][50] = {
    "qsort_test",
    "wcstol_test",
    "strtol_test",
    "strtod_test", 
    "div_test", 
    "fcvt_test", 
    "llabs_test",
    "atof_test",
    "imaxabs_test",
    "lldiv_test",
    "ldiv_test",
    "abs_test",
    "wcstod_test",
    "atoll_test",
    "imaxdiv_test",
    "atoi_test",
    "atol_test",
    "labs_test",
    "ecvt_test", 
    "gcvt_test",
    "bsearch_test",
};

#endif
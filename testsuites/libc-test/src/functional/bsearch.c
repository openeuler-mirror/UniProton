#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "test.h"

#define NELEMS(arr) (sizeof(arr) / sizeof(arr[0]))
#define LENGTH(x) sizeof(x)/sizeof(x[0])

int numeric (const int *p1, const int *p2){
    return(*p1 - *p2);
}

int bsearch_test(void)
{
    /* 数字查找 */
    int *numPtr;
    int keyNum = 512;
    int numarray[] = {123, 145, 512, 627, 800, 933};
    numPtr = (int *)bsearch (&keyNum, numarray, NELEMS(numarray), sizeof(int), (int(*)(const void *,const void *))numeric);
    if (*numPtr != numarray[2])
        t_error("bsearch(\"%d\") want %lld got %d\n", 512, numarray[2], numPtr);
        
    /* 字符串查找 */
    char arr[][10] = {"abc","acb","bac","bca","cab","cba"}; /* 定义二维字符数组*/
    char *keyStr = "bca";/* 要查找的字符串*/
    char *strPtr = NULL; /* 字符指针*/
    strPtr = (char *)bsearch(keyStr,arr,LENGTH(arr),sizeof(arr[0]),(int (*)(const void *,const void *))strcmp);
    if (strPtr == NULL)
        t_error("bsearch(\"%s\"·) want %s got %d\n", keyStr, keyStr, "NULL");

    return t_status;
}
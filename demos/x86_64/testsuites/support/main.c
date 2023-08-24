#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "cpu_config.h"
#include "prt_tick.h"
#include "prt_sys.h"
#include "prt_sem.h"
#include "prt_lapic.h"
#include "tmacros.h"

U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

extern void CpuOfflineHwiCreate(void);
extern void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

#if defined(OS_OPTION_OPENAMP)
int TestOpenamp(void)
{
    int ret;

    ret = rpmsg_service_init();
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}
#endif

void LapicTimerIsr(HwiArg arg)
{
    PRT_TickISR();
}

void testFunc(void)
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif

    Init(0, 0, 0, 0);
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle testTskId;
    struct TskInitParam param = {0};

    void *buf = PRT_MemAlloc(0, 0, 0x2000 + 16);
    if (((uintptr_t)buf & 0xf) != 0) {
        param.stackAddr = ((uintptr_t)buf & (~0xfULL)) + 16;
    } else {
        param.stackAddr = (uintptr_t)buf;
    }

    param.taskEntry = (TskEntryFunc)testFunc;
    param.taskPrio = OS_TSK_PRIORITY_10;
    param.name = "testFunc";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskId, &param);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_TaskResume(testTskId);
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(OS_LAPIC_TIMER, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("line %d, errno: 0x%08x\n", __LINE__, ret);
        return ret;
    }

    ret = PRT_HwiCreate(OS_LAPIC_TIMER, (HwiProcFunc)LapicTimerIsr, 0);
    if (ret != OS_OK) {
        printf("create hwi failed:0x%llx\n", ret);
        return ret;
    }

    OsLapicConfigTick();

    CpuOfflineHwiCreate();

    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

int main(void)
{
    return OsConfigStart();
}

extern int __wrap_memcmp(const void *s1, const void *s2, size_t cnt)
{
    const char *t1 = s1;
    const char *t2 = s2;
    int res = 0;

    while (cnt-- > 0) {
        if (*t1 > *t2) {
            res = 1;
            break;
        } else if (*t1 < *t2) {
            res = -1;
            break;
        } else {
            t1++;
            t2++;
        }
    }
    return res;
}

extern char *__wrap_strncpy(char *dest, const char *src, unsigned int num)
{
    char *ret = dest;
    while (num && *src) {
		*dest++ = *src++;
        num--;
    }
    while (num) {
        *dest++ = '\0';
        num--;
    }

    return ret;
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL || len == 0) {
        return NULL;
    }
    
    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i) {
        ret[i] = set;
    }
    return ret;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
}

extern int __wrap_strcmp(const char *str1, const char *str2)
{
    int ret = 0;
    while (!(ret = *(unsigned char *)str1 - *(unsigned char *)str2) && *str1) {
        str1++;
        str2++;
    }
    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        return 1;
    }
    return 0;
}

extern int __wrap_strncmp(const char *dst, const char *src, int n)
{
    while (--n && (*dst++ == *src++));
    return *dst - *src;
}

extern size_t __wrap_strnlen(const char *str, size_t maxsize)
{
    size_t n;

    for (n = 0; n < maxsize && *str; n++, str++) {
        ;
    }

    return n;
}

extern size_t __wrap_strlen(char *str)
{
    int count = 0;
    while (*str != '\0') {
        count++;
        str++;
    }
    return count;
}

extern void *__wrap_memchr(const void *buf, int c, size_t count)
{
    while (count--) {
        if (*(char *)buf == c) {
            return (void *)buf;
        }
        buf = (char *)buf + 1;
    }
    return NULL;
}

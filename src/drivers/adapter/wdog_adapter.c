#include <nuttx/wdog.h>
#include "prt_sys_external.h"
#include "prt_swtmr_external.h"

static void OsWdogWrapper(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    (void)tmrHandle;
    (void)arg3;
    (void)arg4;
    void *(*wdentry)(wdparm_t) = (void *)arg1;

    wdparm_t val = (wdparm_t)arg2;

    wdentry(val);
}

int wd_start(FAR struct wdog_s *wdog, sclock_t delay, wdentry_t wdentry, wdparm_t arg)
{
    U32 ret;
    struct TimerCreatePara timer = {0};

    if (wdog == NULL || wdentry == NULL || delay < 0) {
        return -EINVAL;
    }

    if (WDOG_ISACTIVE(wdog)) {
        wd_cancel(wdog);
    }
    
    if (delay <= 0) {
        delay = 1;
    } else if (++delay <= 0) {
        delay--;
    }

    timer.type = OS_TIMER_SOFTWARE;
    timer.mode = OS_TIMER_ONCE;
    timer.interval = (U32)DIV64(((U64)delay * OS_SYS_MS_PER_SECOND), OsSysGetTickPerSecond());
    if (DIV64_REMAIN((U64)delay * OS_SYS_MS_PER_SECOND, OsSysGetTickPerSecond()) != 0) {  // 若不整除，则+1
        timer.interval++;
    }
    timer.timerGroupId = 0;
    timer.callBackFunc = OsWdogWrapper;
    timer.arg1 = (U32)wdentry;
    timer.arg2 = (U32)arg;
    ret = PRT_TimerCreate(&timer, &wdog->tmr_handle);
    if (ret != OS_OK) {
        return -EINVAL;
    }

    ret = PRT_TimerStart(0, wdog->tmr_handle);
    if (ret != OS_OK) {
        (void)PRT_TimerDelete(0, wdog->tmr_handle);
        return -EINVAL;
    }
    wdog->is_active = true;
    return OK;
}

int wd_cancel(FAR struct wdog_s *wdog)
{
    if (wdog != NULL && WDOG_ISACTIVE(wdog)) {
        (void)PRT_TimerDelete(0, wdog->tmr_handle);
        wdog->is_active = false;
    }
    return OK;
}

sclock_t wd_gettime(FAR struct wdog_s *wdog)
{
    U32 query, ret;
    if (wdog != NULL && WDOG_ISACTIVE(wdog)) {
        ret = PRT_TimerQuery(0, wdog->tmr_handle, &query);
        if (ret != 0) {
            return -EINVAL;
        }
        return (sclock_t)OS_MS2CYCLE(query, OsSysGetTickPerSecond());
    }
    return -EINVAL;
}
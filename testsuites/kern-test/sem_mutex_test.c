#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_log.h"
#include "prt_sys_external.h"
#include "securec.h"
#include "time.h"
#include "kern_test_public.h"

// #define USE_P_MUTEX

#if defined(USE_P_MUTEX)
#include "pthread.h"
#endif

#define TASK_PRIOR_C 28
#define TASK_PRIOR_B 27
#define TASK_PRIOR_A 26

#if defined(USE_P_MUTEX)
pthread_mutex_t g_test_sem_A = {0};
pthread_mutex_t g_test_sem_B = {0};
#else
SemHandle g_test_sem_A = -1;
SemHandle g_test_sem_B = -1;
#endif

volatile TskHandle g_task_A = -1;
volatile TskHandle g_task_B = -1;
volatile TskHandle g_task_C = -1;

extern void OsTimeGetRealTime(struct timespec *realTime);
void OsTick2Timeout(struct timespec *time, U32 ticks)
{
    OsTimeGetRealTime(time);
    if (ticks == 0) {
        return;
    }

    time->tv_nsec += (ticks % OsSysGetTickPerSecond()) * (OS_SYS_NS_PER_SECOND / OsSysGetTickPerSecond());
    time->tv_sec += ticks / OsSysGetTickPerSecond();

    if (time->tv_nsec >= OS_SYS_NS_PER_SECOND) {
        time->tv_sec += 1;
        time->tv_nsec -= OS_SYS_NS_PER_SECOND; 
    }
    return;
}

#if defined(USE_P_MUTEX)
static inline U32 test_mutex_init(void *lock)
{
    pthread_mutexattr_t attr = {0};
    pthread_mutexattr_init(&attr);
    attr.type = PTHREAD_MUTEX_RECURSIVE;
    return (U32)pthread_mutex_init((pthread_mutex_t *)lock, &attr);
}

static inline U32 test_mutex_lock(void* lock, U32 timeout)
{
    struct timespec time = {0};
    if (timeout == 0) {
        return (U32)pthread_mutex_trylock((pthread_mutex_t *)lock);
    } else if(timeout == OS_WAIT_FOREVER) {
        return (U32)pthread_mutex_lock((pthread_mutex_t *)lock);
    }
    OsTick2Timeout(&time, timeout);
    return pthread_mutex_timedlock((pthread_mutex_t *)lock, &time);
}

static inline U32 test_mutex_unlock(void* lock)
{
    return pthread_mutex_unlock((pthread_mutex_t *)lock);
}

static U32 test_mutex_destroy(void* lock)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)lock;
    if (mutex->magic == 0) {
        return OS_OK;
    }
    return (U32)pthread_mutex_destroy(mutex);
}
#else
static inline U32 test_mutex_init(void *lock)
{
    return PRT_SemMutexCreate((SemHandle *)lock);
}

static inline U32 test_mutex_lock(void* lock, U32 timeout)
{
    return PRT_SemPend(*(SemHandle *)lock, timeout);
}

static inline U32 test_mutex_unlock(void* lock)
{
    return PRT_SemPost(*(SemHandle *)lock);
}

static U32 test_mutex_destroy(void* lock)
{
    U32 ret;
    if ((*(SemHandle *)lock) != -1) {
        ret = PRT_SemDelete(*(SemHandle *)lock);
        if (ret == OS_OK) {
            *(SemHandle *)lock = -1;
        }
        return ret;
    }
    return OS_OK;
}
#endif

static void test_prior_inherit_task_A(void)
{
    U32 ret;
    TskPrior prio;
    TskHandle handle;
    TEST_LOG("[prior_inherit] task A entry, wait for lock");
    PRT_TaskSelf(&handle);
    g_task_A = handle;

    /* 3. 等待互斥锁，并触发优先级继承 */
    ret = test_mutex_lock((void *)&g_test_sem_A, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task A lock fail");
    /* 5. 获取到互斥锁，检查任务C优先级为C，删除任务C，任务B，释放互斥锁，测试结束 */
    TEST_LOG("[prior_inherit] task A get mutex lock A");

    PRT_TaskGetPriority(g_task_C, &prio);
    TEST_IF_ERR_RET_VOID((prio != TASK_PRIOR_C), "[ERROR] task C prior not expected");

    if (g_task_B != -1) PRT_TaskDelete(g_task_B);
    g_task_B = -1;
    if (g_task_C != -1) PRT_TaskDelete(g_task_C);
    g_task_C = -1;

    ret = test_mutex_unlock((void *)&g_test_sem_A);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task A unlock fail");

    g_task_A = -1;
    g_testFinish = 1;
    TEST_LOG("[prior_inherit] test success, task A end, kill task B, task C");
    return;
}

static void test_prior_inherit_task_B(void)
{
    TskHandle handle;
    TEST_LOG("[prior_inherit] task B entry");
    PRT_TaskSelf(&handle);
    g_task_B = handle;

    /* 2. 创建任务A */
    handle = test_start_task((TskEntryFunc)test_prior_inherit_task_A, TASK_PRIOR_A, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task B create task fail");

#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_LOG("[prior_inherit] prio inversion off, task B get sched as expected");
#else
    TEST_LOG("[prior_inherit] ERROR! task B should not get sched in");
    g_testResult = 1;
#endif
    g_testFinish = 1;
    while (1) {}
    return;
}

static void test_prior_inherit_task_C(void) 
{
    U32 ret;
    TskHandle self, handle;
    TskPrior selfPrio;
    void *lockA = (void *)&g_test_sem_A;
    TEST_LOG("[prior_inherit] test start, task C entry");
    PRT_TaskSelf(&self);
    g_task_C = self;

    /* 1. 持有锁，创建任务B */
    ret = test_mutex_init(lockA);
    TEST_IF_ERR_RET_VOID((ret), "[ERROR] init lockA fail");

    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock fail");
    TEST_LOG("[prior_inherit] task C get mutex lock A");

    handle = test_start_task((TskEntryFunc)test_prior_inherit_task_B, TASK_PRIOR_B, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* (如果没开优先级继承，此处B会取代C得到调度） */
    TEST_LOG("[prior_inherit] task C get sched in");
    /* 4. 检查自身优先级为A，释放互斥锁 */
    PRT_TaskGetPriority(self, &selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior not expected");

    TEST_LOG("[prior_inherit] task C going to unlock mutex A");
    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    TEST_LOG("[prior_inherit] ERROR! task C should not get sched in");
    g_testResult = 1;
    g_testFinish = 1;

    while (1) {}
    return;
}

static void test_prior_restore_task_A(void)
{
    U32 ret;
    TskHandle handle;
    TEST_LOG("[prior_restore] task A entry, wait for lock A");
    PRT_TaskSelf(&handle);
    g_task_A = handle;

    /* 4. 等待锁A，触发优先级继承 */
    ret = test_mutex_lock((void *)&g_test_sem_A, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task A lock A fail");
    /* 6. 获取锁A，释放锁A，任务结束 */
    TEST_LOG("[prior_restore] task A get mutex lock A");

    ret = test_mutex_unlock(&g_test_sem_A);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B unlock B fail");
    TEST_LOG("[prior_restore] task A unlock lock A and exit");
    
    g_task_A = -1;
    return;
}

static void test_prior_restore_task_B(void)
{
    U32 ret;
    TskHandle handle;
    TEST_LOG("[prior_restore] task B entry, wait for lock B");
    PRT_TaskSelf(&handle);
    g_task_B = handle;

    /* 2. 等待锁B，触发优先级继承  */
    ret = test_mutex_lock((void *)&g_test_sem_B, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B lock B fail");
    /* 8. 获取锁B，释放锁B，任务结束 */
    TEST_LOG("[prior_restore] task B get mutex lock B");

    ret = test_mutex_unlock(&g_test_sem_B);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B unlock B fail");
    TEST_LOG("[prior_restore] task B unlock lock B and exit");
    
    g_task_B = -1;
    return;
}

static void test_prior_restore_task_C(void)
{
    U32 ret;
    TskHandle self, handle;
    TskPrior selfPrio;
    void *lockA = (void *)&g_test_sem_A;
    void *lockB = (void *)&g_test_sem_B;
    TEST_LOG("[prior_restore] test start, task C entry");
    PRT_TaskSelf(&self);
    g_task_C = self;

    /* 1. 持有锁A 锁B，创建任务B */
    ret = test_mutex_init(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockA fail");

    ret = test_mutex_init(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockB fail");

    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock A fail");
    TEST_LOG("[prior_restore] task C get mutex lock A");

    ret = test_mutex_lock(lockB, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock B fail");
    TEST_LOG("[prior_restore] task C get mutex lock B");

    handle = test_start_task((TskEntryFunc)test_prior_restore_task_B, TASK_PRIOR_B, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    TEST_LOG("[prior_restore] task C get sched in");
    /* 3. 检查自身优先级为B，创建任务A */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_restore] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_B), "[ERROR] task C prior wrong, expected prio B");
    TEST_LOG("[prior_restore] task C check prior B success");
#endif

    handle = test_start_task((TskEntryFunc)test_prior_restore_task_A, TASK_PRIOR_A, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    TEST_LOG("[prior_restore] task C get sched in");
    /* 5. 检查自身优先级为A，嵌套锁A，释放锁A，检查自身优先级为A，释放锁A */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_restore] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio A");
    TEST_LOG("[prior_restore] task C check prior A success");
#endif

#if defined(OS_OPTION_SEM_RECUR_PV)
    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock fail");
    TEST_LOG("[prior_inherit] task C recur lock mutex A");

    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");
    TEST_LOG("[prior_restore] task C recur unlock mutex A");

    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_restore] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio A");
    TEST_LOG("[prior_restore] task C check prior A success");
#endif
#endif

    TEST_LOG("[prior_restore] task C going to unlock mutex A");
    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    TEST_LOG("[prior_restore] task C get sched in");
    /* 7. 检查自身优先级为B，释放锁B */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_restore] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_B), "[ERROR] task C prior wrong, expected prio B");
    TEST_LOG("[prior_restore] task C check prior B success");
#endif

    TEST_LOG("[prior_restore] task C going to unlock mutex B");
    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    TEST_LOG("[prior_restore] task C get sched in");
    /* 9. 检查自身优先级为C，测试结束 */
    PRT_TaskGetPriority(self, &selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_restore] task C check prior C success");

    TEST_LOG("[prior_restore] prior restore test success, task C exit");
    g_task_C = -1;
    g_testFinish = 1;
    return;
}

static void test_tsk_timeout_or_delete_task_A(void)
{
    TskHandle handle;
    TEST_LOG("[timeout_delete] task A entry, wait for lock");
    PRT_TaskSelf(&handle);
    g_task_A = handle;

    /* 4. 等待锁 */
    (void)test_mutex_lock((void *)&g_test_sem_A, OS_WAIT_FOREVER);
    TEST_LOG("[timeout_delete] ERROR! task B should not get sched in");

    g_task_A = -1;
    g_testResult = 1;
    g_testFinish = 1;
    return;
    
}

static void test_tsk_timeout_or_delete_task_B(void)
{
    U32 ret;
    TskHandle handle;
    TEST_LOG("[timeout_delete] task B entry, wait for lock");
    PRT_TaskSelf(&handle);
    g_task_B = handle;

    /* 2. 等待锁，并且等待超时，任务结束 */
    ret = test_mutex_lock((void *)&g_test_sem_A, 1);
#if defined(USE_P_MUTEX)
    TEST_IF_ERR_RET_VOID((ret != ETIMEDOUT), "[ERROR] task B lock A ret wrong, expected ETIMEDOUT");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_SEM_TIMEOUT),
        "[ERROR] task B lock A ret wrong, expected OS_ERRNO_SEM_TIMEOUT");
#endif

    TEST_LOG("[timeout_delete] task B get lock timeout as expected, exit");
    g_task_B = -1;
    return;
}

static void test_tsk_timeout_or_delete_task_C(void)
{
    U32 ret;
    TskHandle self, handle;
    TskPrior selfPrio;
    SemHandle sem;
    U32 pidList[10];
    U32 tskCnt;
    void *lockA = (void *)&g_test_sem_A;

    TEST_LOG("[timeout_delete] test start, task C entry");
    PRT_TaskSelf(&self);
    g_task_C = self;

    /* 1. 持有锁，创建任务B */
    ret = test_mutex_init(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockA fail");

    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock A fail");
    TEST_LOG("[timeout_delete] task C get mutex lock A");

#if defined(USE_P_MUTEX)
    sem = (SemHandle)g_test_sem_A.mutex_sem;
#else
    sem = (SemHandle)g_test_sem_A;
#endif

    handle = test_start_task((TskEntryFunc)test_tsk_timeout_or_delete_task_B, TASK_PRIOR_B, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* 等待任务B超时 */
    PRT_TaskDelay(10);

    TEST_LOG("[timeout_delete] task C get sched in");
    /* 3. 检查pend task数量为0，检查自身优先级为B，创建任务A */
    PRT_SemGetPendList(sem, &tskCnt, pidList, 10);
    TEST_IF_ERR_RET_VOID((tskCnt != 0), "[ERROR] pend task count not expected, expect 0");

    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[timeout_delete] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_B), "[ERROR] task C prior wrong, expected prio B");
    TEST_LOG("[timeout_delete] task C check prior B success");
#endif

    handle = test_start_task((TskEntryFunc)test_tsk_timeout_or_delete_task_A, TASK_PRIOR_A, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    TEST_LOG("[timeout_delete] task C get sched in");
    /* 5. 检查自身优先级为A，检查pend task只有A，删除任务A，检查pend tasks数量为0，自身优先级为A */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[timeout_delete] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio A");
    TEST_LOG("[timeout_delete] task C check prior A success");
#endif

    PRT_SemGetPendList(sem, &tskCnt, pidList, 10);
    TEST_IF_ERR_RET_VOID((tskCnt != 1), "[ERROR] pend task count not expected, expect 1");
    TEST_IF_ERR_RET_VOID((pidList[0] != g_task_A), "[ERROR] expect pend task to be task A");

    ret = PRT_TaskDelete(g_task_A);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] delete task A fail");

    g_task_A = -1;

    PRT_SemGetPendList(sem, &tskCnt, pidList, 10);
    TEST_IF_ERR_RET_VOID((tskCnt != 0), "[ERROR] pend task count not expected, expect 0");
    TEST_LOG("[timeout_delete] task A delete success, pendList clear");

    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[timeout_delete] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio A");
    TEST_LOG("[timeout_delete] task C check prior A success");
#endif

    /* 6.尝试删除锁，失败，释放锁后，检查优先级为C，删除锁成功，测试结束 */
    ret = test_mutex_destroy(lockA);
#if defined(USE_P_MUTEX)
    TEST_IF_ERR_RET_VOID((ret != EINVAL), "[ERROR] task C destory mutex ret wrong expected EINVAL");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_SEM_MUTEX_HOLDING),
        "[ERROR] task C destory mutex ret wrong expected OS_ERRNO_SEM_MUTEX_HOLDING");
#endif
    TEST_LOG("[timeout_delete] task C delete mutex A fail as expected");

    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");
    TEST_LOG("[timeout_delete] task C unlock mutex A");

    PRT_TaskGetPriority(self, &selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[timeout_delete] task C check prior C success");

    ret = test_mutex_destroy(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C destory mutex fail");
    TEST_LOG("[timeout_delete] task C destory mutex A success");

    TEST_LOG("[timeout_delete] timeout delete test success, task C exit");
    g_task_C = -1;
    g_testFinish = 1;
    return;
}

static void test_prior_propagation_task_A(void)
{
    U32 ret;
    TskHandle handle;
    TEST_LOG("[prior_propagation] task A entry, wait for lock");
    PRT_TaskSelf(&handle);
    g_task_A = handle;
    /* 4. 等待锁A */
    ret = test_mutex_lock((void *)&g_test_sem_A, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task A lock fail");
    /* 7. 获取锁A，结束 */
    TEST_LOG("[prior_propagation] task A get mutex lock A and exit");
    return;
}

static void test_prior_propagation_task_B(void)
{
    U32 ret;
    TskHandle handle;
    TskPrior prio;
    void *lockA = (void *)&g_test_sem_A;
    void *lockB = (void *)&g_test_sem_B;
    TEST_LOG("[prior_propagation] task B entry");
    PRT_TaskSelf(&handle);
    g_task_B = handle;

    /* 2. 持有锁A，等待锁B */
    ret = test_mutex_init(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockA fail");

    ret = test_mutex_lock(lockA, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B lock A fail");
    TEST_LOG("[prior_propagation] task B get mutex lock A, wait for lock B");

    ret = test_mutex_lock(lockB, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B lock B fail");
    /* 6. 获取锁B，检查自身优先级为A，检查任务C优先级为C，释放锁A */
    TEST_LOG("[prior_propagation] task B get mutex lock B");

    PRT_TaskGetPriority(handle, &prio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((prio != TASK_PRIOR_B), "[ERROR] task B prior wrong, expected prio B");
    TEST_LOG("[prior_propagation] task B check prior B success");
#else
    TEST_IF_ERR_RET_VOID((prio != TASK_PRIOR_A), "[ERROR] task B prior wrong, expected prio A");
    TEST_LOG("[prior_propagation] task B check prior A success");
#endif

    PRT_TaskGetPriority(g_task_C, &prio);
    TEST_IF_ERR_RET_VOID((prio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_propagation] task B check task C prior C success");

    TEST_LOG("[prior_propagation] task B going to unlock mutex A");
    ret = test_mutex_unlock(lockA);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B unlock fail");

    TEST_LOG("[prior_propagation] task B get sched in");
    /* 8. 检查自身优先级为B，任务循环延迟 */
    PRT_TaskGetPriority(handle, &prio);
    TEST_IF_ERR_RET_VOID((prio != TASK_PRIOR_B), "[ERROR] task B prior wrong, expected prio B");
    TEST_LOG("[prior_propagation] task B check prior B success");

    while (1) {
        PRT_TaskDelay(100);
    }
    return;
}

static void test_prior_propagation_task_C(void)
{
    U32 ret;
    TskHandle self, handle;
    TskPrior selfPrio;
    TskStatus status;
    void *lockA = (void *)&g_test_sem_A;
    void *lockB = (void *)&g_test_sem_B;

    TEST_LOG("[prior_propagation] test start, task C entry");
    PRT_TaskSelf(&self);
    g_task_C = self;

    /* 1. 持有锁B，创建任务B */
    ret = test_mutex_init(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockA fail");

    ret = test_mutex_lock(lockB, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock B fail");
    TEST_LOG("[prior_propagation] task C get mutex lock B");

    handle = test_start_task((TskEntryFunc)test_prior_propagation_task_B, TASK_PRIOR_B, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* 3. 检查自身优先级为B，创建任务A */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_propagation] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_B), "[ERROR] task C prior wrong, expected prio B");
    TEST_LOG("[prior_propagation] task C check prior B success");
#endif

    handle = test_start_task((TskEntryFunc)test_prior_propagation_task_A, TASK_PRIOR_A, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    TEST_LOG("[prior_propagation] task C get sched in");
    /* 5. 检查自身优先级为A（优先级传播），释放锁B */
    PRT_TaskGetPriority(self, &selfPrio);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_C), "[ERROR] task C prior wrong, expected prio C");
    TEST_LOG("[prior_propagation] task C check prior C success");
#else
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio A");
    TEST_LOG("[prior_propagation] task C check prior A success");
#endif
    TEST_LOG("[prior_propagation] task C going to unlock mutex B");
    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    TEST_LOG("[prior_propagation] task C get sched in");
    /* 9. 删除任务B，检查锁A，B均未释放，但任务A，任务B已经正常删除，测试结束 */
    ret = PRT_TaskDelete(g_task_B);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] delete task B fail");

    ret = test_mutex_lock(lockA, 0);
#if defined(USE_P_MUTEX)
    TEST_IF_ERR_RET_VOID((ret != EBUSY),
        "[ERROR] task C lock A ret wrong, expected EBUSY");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_SEM_UNAVAILABLE),
        "[ERROR] task C lock A ret wrong, expected OS_ERRNO_SEM_UNAVAILABLE");
#endif
    TEST_LOG("[prior_propagation] task C get lock A unavaliable as expected");

    ret = test_mutex_lock(lockB, 1);
#if defined(USE_P_MUTEX)
    TEST_IF_ERR_RET_VOID((ret != ETIMEDOUT), "[ERROR] task C lock B ret wrong, expected ETIMEDOUT");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_SEM_TIMEOUT),
        "[ERROR] task C lock B ret wrong, expected OS_ERRNO_SEM_TIMEOUT");
#endif
    TEST_LOG("[prior_propagation] task C get lock B timeout as expected, exit");

    status = PRT_TaskGetStatus(g_task_B);
    TEST_IF_ERR_RET_VOID(status, "[ERROR] task B status not unused");

    status = PRT_TaskGetStatus(g_task_A);
    TEST_IF_ERR_RET_VOID(status, "[ERROR] task A status not unused");
    TEST_LOG("[prior_propagation] task C check task A and task B all unused, exit");

    g_task_B = -1;
    g_task_A = -1;
    g_task_C = -1;
    g_testFinish = 1;
    return;
}

static void test_prior_set_task_B(void)
{
    U32 ret;
    TskHandle self;
    TskPrior selfPrio;
    void *lockB = (void *)&g_test_sem_B;

    TEST_LOG("[prior_set] test start, task B entry");
    PRT_TaskSelf(&self);
    g_task_B = self;

    /* 2. 等待锁B */
    ret = test_mutex_lock(lockB, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B lock B fail");
    /* 4. 获取锁B，等待一段时间 */
    TEST_LOG("[prior_set] task B get mutex lock B");
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    /* 6. 修改自身优先级为C以下，失败（任务C在等待），修改自身优先级为A，成功，释放锁B，检查自身优先级，结束 */
    ret = PRT_TaskSetPriority(self, TASK_PRIOR_C);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B set self prior C fail");
    TEST_LOG("[prior_set] task B set self prior C success");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_TSK_PRIOR_LOWER_THAN_PENDTSK),
        "[ERROR] task B set self prior ret not expected");
    TEST_LOG("[prior_set] task B set self prior fail as expected");
#endif

    ret = PRT_TaskSetPriority(self, TASK_PRIOR_A);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B set self prior fail");
    TEST_LOG("[prior_set] task B set self prior A sccuess");

    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task B unlock B fail");
    TEST_LOG("[prior_set] task B unlock lock B");

    PRT_TaskGetPriority(self, &selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task B prior wrong, expected prio A");
    TEST_LOG("[prior_set] task B check prior A success and exit");

    g_task_B = -1;
    return;
}

static void test_prior_set_task_C(void)
{
    U32 ret;
    TskHandle self, handle;
    TskPrior selfPrio;
    void *lockB = (void *)&g_test_sem_B;

    TEST_LOG("[prior_set] test start, task C entry");
    PRT_TaskSelf(&self);
    g_task_C = self;

    /* 1. 持有锁B，创建任务B */
    ret = test_mutex_init(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lockA fail");

    ret = test_mutex_lock(lockB, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock B fail");
    TEST_LOG("[prior_set] task C get mutex lock B");

    handle = test_start_task((TskEntryFunc)test_prior_set_task_B, TASK_PRIOR_B, OS_TSK_SCHED_FIFO);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* 3. 修改B优先级，失败（等待互斥），修改自身优先级，失败（继承），释放锁B */
    ret = PRT_TaskSetPriority(g_task_B, TASK_PRIOR_A);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C set task B prior A fail");
    TEST_LOG("[prior_set] task C set task B prior A success");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_TSK_PEND_MUTEX), "[ERROR] task C set task B prior ret not expected");
    TEST_LOG("[prior_set] task C set task B prior fail as expected");
#endif

    ret = PRT_TaskSetPriority(self, TASK_PRIOR_B);
#if !defined(OS_OPTION_SEM_PRIO_INHERIT)
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C set self prior B fail");
    TEST_LOG("[prior_set] task C set self prior B success");
#else
    TEST_IF_ERR_RET_VOID((ret != OS_ERRNO_TSK_PRIORITY_INHERIT), "[ERROR] task C set self prior ret not expected");
    TEST_LOG("[prior_set] task C set self prior fail as expected");
#endif

    TEST_LOG("[prior_set] task C going to unlock mutex B");
    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    /* 5. 等待锁B */
    ret = test_mutex_lock(lockB, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock B fail");
    /* 7. 获取锁B，修改自身优先级为A，成功，释放锁B，检查自身优先级，修改自身优先级为C，成功，结束 */
    TEST_LOG("[prior_set] task C get mutex lock B");

    ret = PRT_TaskSetPriority(self, TASK_PRIOR_A);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C set self prior A fail");
    TEST_LOG("[prior_set] task C set self prior A sccuess");

    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock B fail");
    TEST_LOG("[prior_set] task C unlock lock B");

    ret = PRT_TaskGetPriority(self, &selfPrio);
    TEST_IF_ERR_RET_VOID((selfPrio != TASK_PRIOR_A), "[ERROR] task C prior wrong, expected prio B");
    TEST_LOG("[prior_set] task C check prior A success");

    ret = PRT_TaskSetPriority(self, TASK_PRIOR_C);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C set self prior C fail");
    TEST_LOG("[prior_set] task C set self prior C sccuess and exit");

    g_task_A = -1;
    g_testFinish = 1;
    return;
}

char g_test_pior_list[4][3] = {"B1", "A1", "B2", "A2"};

static void test_prior_pend_list_task_param(uintptr_t index, uintptr_t taskLeft, uintptr_t arg3, uintptr_t arg4)
{
    U32 ret, tskCnt;
    TskHandle self;
    SemHandle sem;
    U32 pidList[10];
    void *lockB = (void *)&g_test_sem_B;
    char *name = g_test_pior_list[index];
    (void)arg3;
    (void)arg4;
    TEST_LOG_FMT("[prior_pend_list] task %s entry, wait for lock", name);
    PRT_TaskSelf(&self);
#if defined(USE_P_MUTEX)
    sem = (SemHandle)g_test_sem_B.mutex_sem;
#else
    sem = (SemHandle)g_test_sem_B;
#endif

    /* 等待锁 */
    ret = test_mutex_lock(lockB, OS_WAIT_FOREVER);
    TEST_IF_ERR_RET_VOID_FMT(ret, "[ERROR] task %s lock fail", name);
    /* 获取锁，检查等待队列长度为[taskLeft]，释放锁，结束*/
    TEST_LOG_FMT("[prior_pend_list] task %s get mutex lock", name);

    PRT_SemGetPendList(sem, &tskCnt, pidList, 10);
    TEST_IF_ERR_RET_VOID_FMT((tskCnt != taskLeft), "[ERROR] pend task count:%u, expect:%u", tskCnt, (U32)taskLeft);
    TEST_LOG_FMT("[prior_pend_list] task %s check pend list success", name);

    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");
    TEST_LOG_FMT("[prior_pend_list] task %s unlock lock and exit", name);
    return;
}

static void test_prior_pend_list_task_C(void)
{
    U32 ret;
    TskHandle self, handle;
    void *lockB = (void *)&g_test_sem_B;
    TEST_LOG("[prior_pend_list] test start, task C entry");
    PRT_TaskSelf(&self);

    /* 持有锁，创建任务B */
    ret = test_mutex_init(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] init lock fail");

    ret = test_mutex_lock(lockB, 0);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C lock fail");
    TEST_LOG("[prior_pend_list] task C get mutex lock");

    /* 创建任务B1，B1获取到锁后，等待队列长度应该为1 */
    handle = test_start_task_param((TskEntryFunc)test_prior_pend_list_task_param, TASK_PRIOR_B, OS_TSK_SCHED_FIFO, 0, 1, 0, 0);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* 创建任务A1，A1获取到锁后，等待队列长度应该为3 */
    handle = test_start_task_param((TskEntryFunc)test_prior_pend_list_task_param, TASK_PRIOR_A, OS_TSK_SCHED_FIFO, 1, 3, 0, 0);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    /* 创建任务B2，B2获取到锁后，等待队列长度应该为0 */
    handle = test_start_task_param((TskEntryFunc)test_prior_pend_list_task_param, TASK_PRIOR_B, OS_TSK_SCHED_FIFO, 2, 0, 0, 0);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    // 此时任务C优先级可能已经提升为A，需要等待一段时间后创建任务A2
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    /* 创建任务A2，A2获取到锁后，等待队列长度应该为2 */
    handle = test_start_task_param((TskEntryFunc)test_prior_pend_list_task_param, TASK_PRIOR_A, OS_TSK_SCHED_FIFO, 3, 2, 0, 0);
    TEST_IF_ERR_RET_VOID((handle == -1), "[ERROR] task C create task fail");

    // 此时任务C优先级可能已经提升为A，需要等待一段时间后释放锁
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    TEST_LOG("[prior_pend_list] task C going to unlock mutex");
    ret = test_mutex_unlock(lockB);
    TEST_IF_ERR_RET_VOID(ret, "[ERROR] task C unlock fail");

    /* 测试结束 */
    TEST_LOG("[prior_pend_list] task C exit, test finish");
    g_testFinish = 1;
    return;
}

// 默认从任务C拉起
static int prt_sem_mutex_test_start(TskEntryFunc func)
{
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    g_testFinish = 0;
    g_testResult = 0;
    g_task_A = -1;
    g_task_B = -1;
    g_task_C = -1;

    g_task_C = test_start_task(func, TASK_PRIOR_C, OS_TSK_SCHED_FIFO);
    if (g_task_C == -1) {
        return -1;
    }

    while (g_testFinish == 0) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }

    if (g_task_C != -1) PRT_TaskDelete(g_task_C);
    if (g_task_B != -1) PRT_TaskDelete(g_task_B);
    if (g_task_A != -1) PRT_TaskDelete(g_task_A);

    (void)test_mutex_destroy((void *)&g_test_sem_A);
    (void)test_mutex_destroy((void *)&g_test_sem_B);
    return g_testResult;
}

/**
 * 标准优先级反转场景
 * 三个任务，任务优先级：A > B > C
 * 执行顺序如下：
 * C 持有锁，创建任务B
 * B 创建任务A
 * A 等待互斥锁，并触发优先级继承
 * (如果没开优先级继承，此处B会取代C得到调度）
 * C 检查自身优先级为A，释放互斥锁
 * A 获取到互斥锁，检查任务C优先级为C，删除任务C，任务B，释放互斥锁，测试结束
*/
static int test_prior_inherit(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_prior_inherit_task_C);
}

/**
 * 优先级继承并恢复，锁嵌套
 * 三个任务，任务优先级：A > B > C
 * 执行顺序如下：
 * C 持有锁A 锁B，创建任务B
 * B 等待锁B，触发优先级继承 
 * C 检查自身优先级为B，创建任务A
 * A 等待锁A，触发优先级继承
 * C 检查自身优先级为A，嵌套锁A，释放锁A，检查自身优先级为A，释放锁A
 * A 获取锁A，释放锁A，任务结束
 * C 检查自身优先级为B，释放锁B
 * B 获取锁B，释放锁B，任务结束
 * C 检查自身优先级为C，测试结束
 * （如果没开优先级继承，执行流程是一致的，但是优先级不会变化）
*/
static int test_prior_restore(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_prior_restore_task_C);
}

/**
 * 等待中的任务等待超时，或被删除，持有锁的任务不会恢复优先级
 * 三个任务，任务优先级：A > B > C
 * 执行顺序如下：
 * C 持有锁，创建任务B
 * B 等待锁，并且等待超时，任务结束
 * C 检查pend task数量为0，检查自身优先级为B，创建任务A
 * A 等待锁
 * C 检查自身优先级为A，检查pend task只有A，删除任务A，检查pend tasks数量为0，自身优先级为A
 * C 尝试删除锁，失败，释放锁后，检查优先级为C，删除锁成功，测试结束
 * （如果没开优先级继承，执行流程是一致的，但是优先级不会变化）
*/
static int test_tsk_timeout_or_delete(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_tsk_timeout_or_delete_task_C);
}

/**
 * 优先级继承传播，任务删除，自删除测试
 * 三个任务，任务优先级：A > B > C
 * 执行顺序如下：
 * C 持有锁B，创建任务B
 * B 持有锁A，等待锁B
 * C 检查自身优先级为B，创建任务A
 * A 等待锁A
 * C 检查自身优先级为A（优先级传播），释放锁B
 * B 获取锁B，检查自身优先级为A，检查任务C优先级为C，释放锁A
 * A 获取锁A，结束
 * B 检查自身优先级为B，任务循环延迟
 * C 删除任务B，检查锁A，B均未释放，但任务A，任务B已经正常删除，测试结束
 * （如果没开优先级继承，执行流程是一致的，但是优先级不会变化）
*/
static int test_prior_propagation(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_prior_propagation_task_C);
}

/**
 * 优先级设置测试，在特定情况下不能修改任务优先级
 * 两个任务，任务优先级：B > C
 * 执行顺序如下： 
 * C 持有锁B，创建任务B
 * B 等待锁B
 * C 修改B优先级，失败（等待互斥），修改自身优先级，失败（继承），释放锁B
 * B 获取锁B，等待一段时间
 * C 等待锁B
 * B 修改自身优先级为C以下，失败（任务C在等待），修改自身优先级为A，成功，释放锁B，检查自身优先级，结束
 * C 获取锁B，修改自身优先级为A，成功，释放锁B，检查自身优先级，修改自身优先级为C，成功，结束
*/
static int test_prior_set(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_prior_set_task_C);
}

/**
 * 优先级等待队列，优先唤醒优先级高的任务，同优先级按照FIFO唤醒
 * 5个任务，A1 = A2 > B1 = B2 > C
 * 执行顺序如下：
 * C 持有锁，创建任务B1
 * B1 等待锁
 * C 创建任务A1
 * A1 等待锁
 * C 创建任务B2, delay
 * B2 等待锁
 * C 创建任务A2, delay
 * A2 等待锁
 * C 释放锁
 * A1 获取到锁，检查等待队列长度为3，释放锁，结束
 * A2 获取到锁，检查等待队列长度为2，释放锁，结束
 * B1 获取到锁，检查等待队列长度为1，释放锁，结束
 * B2 获取到锁，检查等待队列长度为0，释放锁，结束
 * C 测试结束
*/
static int test_prior_pend_list(void) {
    return prt_sem_mutex_test_start((TskEntryFunc)test_prior_pend_list_task_C);
}

test_case_t g_cases[] = {
    TEST_CASE_Y(test_prior_inherit),
    TEST_CASE_Y(test_prior_restore),
    TEST_CASE_Y(test_tsk_timeout_or_delete),
    TEST_CASE_Y(test_prior_propagation),
    TEST_CASE_Y(test_prior_set),
    TEST_CASE_Y(test_prior_pend_list),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    TEST_LOG("sem mutex test finished\n");
}

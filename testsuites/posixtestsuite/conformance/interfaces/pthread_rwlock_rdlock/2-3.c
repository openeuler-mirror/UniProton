/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_rdlock(pthread_rwlock_t *pthread_rwlock_rdlock_2_3_rwlock)
 *
 * 	If the Thread Execution Scheduling option is supported, 
 *	and the threads involved in the lock are executing with the 
 *	scheduling policies SCHED_FIFO or SCHED_RR, the calling thread shall not
 *	acquire the lock if a writer holds the lock or if writers of 
 *	higher or equal priority are blocked on the lock; 
 *	otherwise, the calling thread shall acquire the lock.
 *
 * In this case, reader has higher priority than the writer
 *
 Steps:
 * We have three threads, main(also a reader), writer, reader
 *
 * 1.  Main thread set its shcedule policy as "SCHED_FIFO", with highest priority 
 *     the three: sched_get_priority_min()+2.
 * 2.  Main thread read lock 'pthread_rwlock_rdlock_2_3_rwlock'
 * 3.  Create a writer thread, with schedule policy as "SCHED_FIFO", and priority
 * 	using sched_get_priority_min().
 * 4.  The thread write lock 'pthread_rwlock_rdlock_2_3_rwlock', should block.
 * 5.  Main thread create a reader thread, with schedule policy as "SCHED_FIFO", and
 *     priority sched_get_priority_min()+1
 * 6.  Reader thread read lock 'pthread_rwlock_rdlock_2_3_rwlock', shouldn't block, since it has a higher priority
 *     than the writer blocked on 'pthread_rwlock_rdlock_2_3_rwlock'
 * 7.  Main thread release the 'pthread_rwlock_rdlock_2_3_rwlock', the writer should get the lock first.
 */

/* NOTE: The test result is UNSUPPORTED if Thread Execution Scheduling option
 * 	 is not supported.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include "posixtest.h"

#define TRD_POLICY SCHED_FIFO

static pthread_rwlock_t pthread_rwlock_rdlock_2_3_rwlock;
static int pthread_rwlock_rdlock_2_3_rd_thread_state;
static int pthread_rwlock_rdlock_2_3_wr_thread_state; 

/* thread states: 
	1: not in thread yet; 
	2: just entered thread ;
	3: just before thread exits;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static int pthread_rwlock_rdlock_2_3_set_priority(pthread_t pid, unsigned policy, unsigned prio)
{
        struct sched_param sched_param;
        memset(&sched_param, 0, sizeof(sched_param));
        sched_param.sched_priority = prio;
        if (pthread_setschedparam(pid, policy, &sched_param) == -1)
        {
                printf("Can't set policy to %d and prio to %d\n",
                        policy, prio);
                return PTS_UNRESOLVED ;
        }
        return 0;
}

static void* pthread_rwlock_rdlock_2_3_fn_rd(void *arg)
{ 
	int rc = 0;
	int priority;
	pthread_rwlock_rdlock_2_3_rd_thread_state = ENTERED_THREAD;
	
	priority = (int)(long)arg;
	pthread_rwlock_rdlock_2_3_set_priority(pthread_self(), TRD_POLICY, priority);

	printf("rd_thread: attempt read lock\n");
	rc = pthread_rwlock_rdlock(&pthread_rwlock_rdlock_2_3_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: rd_thread failed to get read lock, Error code:%d\n"
			, rc);
		return PTS_FAIL ;
	} else
		printf("rd_thread: acquired read lock\n");


	sleep(1);

	printf("rd_thread: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_2_3_rwlock) != 0)
	{
		printf("rd_thread: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_rdlock_2_3_rd_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
static void* pthread_rwlock_rdlock_2_3_fn_wr(void *arg)
{ 
	int rc = 0;
	int priority;
	pthread_rwlock_rdlock_2_3_wr_thread_state = ENTERED_THREAD;
	
	priority = (long)arg;
	pthread_rwlock_rdlock_2_3_set_priority(pthread_self(), TRD_POLICY, priority);

	printf("wr_thread: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&pthread_rwlock_rdlock_2_3_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: wr_thread failed to get write lock, Error code:%d\n"
			, rc);
		return PTS_UNRESOLVED ;
	} else
		printf("wr_thread: acquired write lock\n");

	sleep(1);

	printf("wr_thread: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_2_3_rwlock) != 0)
	{
		printf("wr_thread: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_rdlock_2_3_wr_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_rdlock_2_3()
{

	#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
  		printf("Posix Thread Execution Scheduling not supported\n");
  		return PTS_UNSUPPORTED;
	#endif

	int cnt = 0;
	pthread_t rd_thread, wr_thread;
	int priority;
	
	/* main thread needs to have the highest priority*/
	priority = sched_get_priority_min(TRD_POLICY) + 2;
	pthread_rwlock_rdlock_2_3_set_priority(pthread_self(), TRD_POLICY, priority);

	if(pthread_rwlock_init(&pthread_rwlock_rdlock_2_3_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&pthread_rwlock_rdlock_2_3_rwlock) != 0)
	{
		printf("Test FAILED: main cannot get read lock when no one owns the lock\n");
		return PTS_FAIL;
	} else
		printf("main: acquired read lock\n");
	
	pthread_rwlock_rdlock_2_3_wr_thread_state = NOT_CREATED_THREAD;
	priority = sched_get_priority_min(TRD_POLICY);
	printf("main: create wr_thread, with priority: %d\n", priority);
	if(pthread_create(&wr_thread, NULL, pthread_rwlock_rdlock_2_3_fn_wr, (void*)(long)priority) != 0)
	{
		printf("main: Error at 1st pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	
	/* We expect the wr_thread to block */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_2_3_wr_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_2_3_wr_thread_state == EXITING_THREAD )
	{
		printf("wr_thread did not block on write lock, when a reader owns the lock\n");
		return PTS_UNRESOLVED ;
	}
	else if(pthread_rwlock_rdlock_2_3_wr_thread_state != ENTERED_THREAD)
	{
		printf("Unexpected wr_thread state\n");
		return PTS_UNRESOLVED ;
	}
	
	pthread_rwlock_rdlock_2_3_rd_thread_state = NOT_CREATED_THREAD;
	priority = sched_get_priority_min(TRD_POLICY) + 1;
	printf("main: create rd_thread, with priority: %d\n", priority);
	if(pthread_create(&rd_thread, NULL, pthread_rwlock_rdlock_2_3_fn_rd, (void*)(long)priority) != 0)
	{
		printf("main: failed at creating rd_thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* We expect the rd_thread get the read lock*/
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_2_3_rd_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_2_3_rd_thread_state == ENTERED_THREAD )
	{
		printf("Test FAILED: rd_thread blocked on read lock\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_rdlock_2_3_rd_thread_state != EXITING_THREAD)
	{
		printf("Unexpected rd_thread state\n");
		return PTS_UNRESOLVED ;
	}

	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_2_3_rwlock) != 0)
	{
		printf("main: failed to release read lock\n");
		return PTS_UNRESOLVED ;
	}
	
	/* we expect the writer get the lock */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_2_3_wr_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_2_3_wr_thread_state == ENTERED_THREAD )
	{
		printf("Test FAILED: higher priority wr_thread still blocked on write lock, when a reader release the lock\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_rdlock_2_3_wr_thread_state != EXITING_THREAD)
	{
		printf("Unexpected wr_thread state\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_join(wr_thread, NULL) != 0)
	{
		printf("main: Error at 1st pthread_join()\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_join(rd_thread, NULL) != 0)
	{
		printf("main: Error at 2nd pthread_join()\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_rdlock_2_3_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}

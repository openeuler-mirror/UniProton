/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_rwlock_trywrlock(pthread_rwlock_t *pthread_rwlock_trywrlock_1_1_rwlock)
 *
 * The function shall apply a write lock like the pthread_rwlock_wrlock(), with the exception 
 * that the funciton shall fail if any thread currently holds pthread_rwlock_trywrlock_1_1_rwlock(for reading and writing).
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_trywrlock_1_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'pthread_rwlock_trywrlock_1_1_rwlock' for reading with pthread_rwlock_rdlock()
 * 3.  Create a child thread, the thread locks 'pthread_rwlock_trywrlock_1_1_rwlock' for writing, using 
 *     pthread_rwlock_trywrlock(), should get EBUSY
 * 4.  Main thread unlocks 'pthread_rwlock_trywrlock_1_1_rwlock'
 * 5.  Main thread locks 'pthread_rwlock_trywrlock_1_1_rwlock' for writing, using pthread_rwlock_trywrlock(), 
 *     should get the lock successfully
 * 6.  Create child thread to lock 'pthread_rwlock_trywrlock_1_1_rwlock' for writing, with pthread_rwlock_trywrlock(), 
 *     should get EBUSY
 * 7.  Main thread unlock 'pthread_rwlock_trywrlock_1_1_rwlock'
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t pthread_rwlock_trywrlock_1_1_rwlock;
static int pthread_rwlock_trywrlock_1_1_thread_state;
static int pthread_rwlock_trywrlock_1_1_get_ebusy; 

/* pthread_rwlock_trywrlock_1_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_trywrlock_1_1_fn_wr(void *arg)
{ 
	pthread_rwlock_trywrlock_1_1_thread_state = ENTERED_THREAD;
	int rc;

	printf("thread: attempt pthread_rwlock_trywrlock()\n");	
	rc = pthread_rwlock_trywrlock(&pthread_rwlock_trywrlock_1_1_rwlock);
	if(rc != EBUSY)
	{
		printf("Test FAILED: thread: Expected EBUSY, got %d\n", rc);
		return PTS_FAIL ;
	}
	pthread_rwlock_trywrlock_1_1_get_ebusy = 1;
	printf("thread: correctly got EBUSY\n");
	pthread_rwlock_trywrlock_1_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_trywrlock_1_1()
{
	int cnt = 0;
	int rc = 0;
	pthread_t thread1, thread2;
	
	pthread_rwlock_trywrlock_1_1_get_ebusy = 0;
	
	if(pthread_rwlock_init(&pthread_rwlock_trywrlock_1_1_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&pthread_rwlock_trywrlock_1_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: acquired read lock\n");
	pthread_rwlock_trywrlock_1_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create thread1\n");
	if(pthread_create(&thread1, NULL, pthread_rwlock_trywrlock_1_1_fn_wr, NULL) != 0)
	{
		printf("Error creating thread1\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	/* We do no expect thread1 to block */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_trywrlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_trywrlock_1_1_thread_state == ENTERED_THREAD)
	{
		/* the child thread blocked */
		printf("Test FAILED: thread1 should not block on pthread_rwlock_trywrlock()\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_trywrlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state for thread1: %d\n", pthread_rwlock_trywrlock_1_1_thread_state);
		return PTS_UNRESOLVED ;
	}
		
	if(pthread_rwlock_trywrlock_1_1_get_ebusy != 1)
	{
		printf("Test FAILED: thread1 should get EBUSY\n");
		return PTS_FAIL ;
	}

	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_trywrlock_1_1_rwlock) != 0)
	{
		printf("main: Error releasing read lock\n");
		return PTS_UNRESOLVED ;
	}
		
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("main: Error joining thread1\n");
		return PTS_UNRESOLVED ;
	}
	
	printf("main: attempt pthread_rwlock_trywrlock()\n");
	/* Should get the write lock*/
	rc = pthread_rwlock_trywrlock(&pthread_rwlock_trywrlock_1_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: main failed at pthread_rwlock_trywrlock(), error code: %d\n", rc);
		return PTS_FAIL;	
	}

	printf("main: acquired write lock\n");

	pthread_rwlock_trywrlock_1_1_get_ebusy = 0;
	pthread_rwlock_trywrlock_1_1_thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create thread2\n");
	if(pthread_create(&thread2, NULL, pthread_rwlock_trywrlock_1_1_fn_wr, NULL) != 0)
	{
		printf("main: Error creating thread2\n");
		return PTS_UNRESOLVED;
	}
	
	/* We do not expect thread2 to block */
	do{
		sleep(1);
	}while (pthread_rwlock_trywrlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_trywrlock_1_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread2 should not block on pthread_rwlock_trywrlock()\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_trywrlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state: %d\n", pthread_rwlock_trywrlock_1_1_thread_state);
		return PTS_UNRESOLVED ;
	}

	if(pthread_rwlock_trywrlock_1_1_get_ebusy != 1)
	{
		printf("Test FAILED: thread2 should get EBUSY\n");
		return PTS_FAIL ;
	}
	
	printf("main: unlock write lock\n");
	pthread_rwlock_trywrlock_1_1_thread_state = 1;
	if(pthread_rwlock_unlock(&pthread_rwlock_trywrlock_1_1_rwlock) != 0)
	{
		printf("main: Error at 2nd pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_join(thread2, NULL) != 0)
	{
		printf("main: Error at 2cn pthread_join()\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_rwlock_destroy(&pthread_rwlock_trywrlock_1_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}

/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutexattr_settype()
 *
 * PTHREAD_MUTEX_ERRORCHECK

 * Provides errorchecking.  A thread attempting to relock this pthread_mutexattr_settype_3_2_mutex without unlocking it 
 * first will return with an error.  A thread attempting to unlock a pthread_mutexattr_settype_3_2_mutex which another
 * thread has locked will return with an error.  A thread attempting to unlock an unlocked
 * pthread_mutexattr_settype_3_2_mutex will return with an error.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2   Set the 'type' of the mutexattr object to PTHREAD_MUTEX_ERRORCHECK.
 * 3.  Create a pthread_mutexattr_settype_3_2_mutex with that mutexattr object.
 * 4.  Lock the pthread_mutexattr_settype_3_2_mutex. 
 * 5.  Create a thread, and in that thread, attempt to unlock the pthread_mutexattr_settype_3_2_mutex. It should return an
 *     error.
 * 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_t pthread_mutexattr_settype_3_2_thread1;
pthread_mutex_t pthread_mutexattr_settype_3_2_mutex;
pthread_mutexattr_t pthread_mutexattr_settype_3_2_mta;

int pthread_mutexattr_settype_3_2_ret;			/* Return value of the thread unlocking the pthread_mutexattr_settype_3_2_mutex. */

void *pthread_mutexattr_settype_3_2_a_thread_func()
{
	/* Try to unlock the pthread_mutexattr_settype_3_2_mutex that main already locked. */
	pthread_mutexattr_settype_3_2_ret=pthread_mutex_unlock(&pthread_mutexattr_settype_3_2_mutex);
	pthread_exit((void*)0);
}


int pthread_mutexattr_settype_3_2()
{
	
	/* Initialize a pthread_mutexattr_settype_3_2_mutex attributes object */
	if(pthread_mutexattr_init(&pthread_mutexattr_settype_3_2_mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* Set the 'type' attribute to be PTHREAD_MUTEX_ERRORCHECK  */
	if(pthread_mutexattr_settype(&pthread_mutexattr_settype_3_2_mta, PTHREAD_MUTEX_ERRORCHECK) != 0)
	{
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}

	/* Initialize the pthread_mutexattr_settype_3_2_mutex with that attribute obj. */	
	if(pthread_mutex_init(&pthread_mutexattr_settype_3_2_mutex, &pthread_mutexattr_settype_3_2_mta) != 0)
	{
		perror("Error intializing the pthread_mutexattr_settype_3_2_mutex.\n");
		return PTS_UNRESOLVED;
	}

	/* Lock the pthread_mutexattr_settype_3_2_mutex. */
	if(pthread_mutex_lock(&pthread_mutexattr_settype_3_2_mutex) != 0 )
	{
		perror("Error locking the pthread_mutexattr_settype_3_2_mutex first time around.\n");
		return PTS_UNRESOLVED;
	}

	/* Create the thread that will try to unlock the pthread_mutexattr_settype_3_2_mutex we just locked. */
	if(pthread_create(&pthread_mutexattr_settype_3_2_thread1, NULL, pthread_mutexattr_settype_3_2_a_thread_func, NULL) != 0)
	{
		perror("Error creating a thread.\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for that thread to end execution */
	pthread_join(pthread_mutexattr_settype_3_2_thread1, NULL);

	if(pthread_mutexattr_settype_3_2_ret == 0)
	{
		printf("Test FAILED: Expected an error when trying to unlock a pthread_mutexattr_settype_3_2_mutex that was locked by another thread.  Returned 0 instead.\n");
		return PTS_FAIL;
	}
	
	/* cleanup */
	pthread_mutex_unlock(&pthread_mutexattr_settype_3_2_mutex);	
	pthread_mutex_destroy(&pthread_mutexattr_settype_3_2_mutex);
	
	if(pthread_mutexattr_destroy(&pthread_mutexattr_settype_3_2_mta))
	{
		perror("Error at pthread_mutexattr_destory().\n");
		return PTS_UNRESOLVED;
	}	
		
	printf("Test PASSED\n");
	return PTS_PASS;
}

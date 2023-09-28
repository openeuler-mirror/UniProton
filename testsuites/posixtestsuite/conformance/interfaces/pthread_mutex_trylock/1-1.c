/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_trylock()
 *   is equivalent to pthread_mutex_lock() except that if the pthread_mutex_trylock_1_1_mutex object 
 *   referenced by 'pthread_mutex_trylock_1_1_mutex' is currently locked (by any thread, including the
 *   current thread), the call shall return immediately.

 * Steps: 
 *   -- Initilize a pthread_mutex_trylock_1_1_mutex object
 *   -- Create a secondary thread and have it lock the pthread_mutex_trylock_1_1_mutex
 *   -- Within the main thread, try to lock the pthread_mutex_trylock_1_1_mutex using 
 	pthread_mutex_trylock() and EBUSY is expected
 *   -- Have the secondary thread unlock the pthread_mutex_trylock_1_1_mutex
 *   -- Within the main thread, try to lock the pthread_mutex_trylock_1_1_mutex again  
 	and expect a successful locking.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

void *pthread_mutex_trylock_1_1_func(void *parm);

pthread_mutex_t    pthread_mutex_trylock_1_1_mutex = PTHREAD_MUTEX_INITIALIZER;
int     pthread_mutex_trylock_1_1_t1_start=0;
int	pthread_mutex_trylock_1_1_t1_pause=1;

int pthread_mutex_trylock_1_1()
{
  	int           	i, rc;
  	pthread_t       t1;

	/* Create a secondary thread and wait until it has locked the pthread_mutex_trylock_1_1_mutex */
    	pthread_create(&t1, NULL, pthread_mutex_trylock_1_1_func, NULL);
		// 增加调度点
		sched_yield();
    	while(!pthread_mutex_trylock_1_1_t1_start)
		sleep(1);
    		
	/* Trylock the pthread_mutex_trylock_1_1_mutex and expect it returns EBUSY */
   	rc = pthread_mutex_trylock(&pthread_mutex_trylock_1_1_mutex);
      	if(rc!=EBUSY) {
        	printf("Expected %d(EBUSY), got %d\n",EBUSY,rc);
        	printf("Test FAILED\n");
		return PTS_FAIL;
      	}
    	
    	/* Allow the secondary thread to go ahead */
	pthread_mutex_trylock_1_1_t1_pause=0;
	
	/* Trylock the pthread_mutex_trylock_1_1_mutex for N times */
	for(i=0; i<5; i++) {
		rc = pthread_mutex_trylock(&pthread_mutex_trylock_1_1_mutex);
		if(rc==0) {
			pthread_mutex_unlock(&pthread_mutex_trylock_1_1_mutex);
			break;
		}
		else if(rc==EBUSY) {
			sleep(1);
			continue;
		}
		else {
			printf("Unexpected error code(%d) for pthread_mutex_lock()\n", rc);
			return PTS_UNRESOLVED;
		}
	}
		
	/* Clean up */
	pthread_join(t1, NULL);
  	pthread_mutex_destroy(&pthread_mutex_trylock_1_1_mutex);

	if(i>=5) {
		printf("Have tried %d times but failed to get the pthread_mutex_trylock_1_1_mutex\n", i);
		return PTS_UNRESOLVED;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}

void *pthread_mutex_trylock_1_1_func(void *parm)
{
  	int rc;

	if((rc=pthread_mutex_lock(&pthread_mutex_trylock_1_1_mutex))!=0) {
		printf("Error at pthread_mutex_lock(), rc=%d\n",rc);
		pthread_exit((void*)PTS_UNRESOLVED);
	}
	pthread_mutex_trylock_1_1_t1_start=1;
	
	while(pthread_mutex_trylock_1_1_t1_pause)
		sleep(1);

	if((rc=pthread_mutex_unlock(&pthread_mutex_trylock_1_1_mutex))!=0) {
		printf("Error at pthread_mutex_unlock(), rc=%d\n",rc);
		pthread_exit((void*)PTS_UNRESOLVED);
	}

  	pthread_exit(0);
  	return (void*)(0);
}

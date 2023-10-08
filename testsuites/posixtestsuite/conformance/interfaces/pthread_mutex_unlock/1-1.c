/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_unlock()
 *   shall release the pthread_mutex_unlock_1_1_mutex object 'pthread_mutex_unlock_1_1_mutex'.

 * Steps: 
 *   -- Initilize a pthread_mutex_unlock_1_1_mutex object
 *   -- Get the pthread_mutex_unlock_1_1_mutex using pthread_mutex_lock()
 *   -- Release the pthread_mutex_unlock_1_1_mutex using pthread_mutex_unlock()
 *   -- Try to get the pthread_mutex_unlock_1_1_mutex using pthread_mutex_trylock()
 *   -- Release the pthread_mutex_unlock_1_1_mutex using pthread_mutex_unlock()
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_mutex_t    pthread_mutex_unlock_1_1_mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_unlock_1_1()
{
  	int  rc;

	/* Get the pthread_mutex_unlock_1_1_mutex using pthread_mutex_lock() */
	if((rc=pthread_mutex_lock(&pthread_mutex_unlock_1_1_mutex)) != 0) {
		printf("Error at pthread_mutex_lock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Release the pthread_mutex_unlock_1_1_mutex using pthread_mutex_unlock() */
	if((rc=pthread_mutex_unlock(&pthread_mutex_unlock_1_1_mutex)) != 0) {
        	printf("pthread_mutex_unlock 1 failed!\n");
        	printf("Test FAILED\n");
		return PTS_FAIL;
	}
    		
	/* Get the pthread_mutex_unlock_1_1_mutex using pthread_mutex_trylock() */
	if((rc=pthread_mutex_trylock(&pthread_mutex_unlock_1_1_mutex)) != 0) {
        	printf("pthread_mutex_trylock failed!\n");
        	printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Release the pthread_mutex_unlock_1_1_mutex using pthread_mutex_unlock() */
	if((rc=pthread_mutex_unlock(&pthread_mutex_unlock_1_1_mutex)) != 0) {
        	printf("pthread_mutex_unlock 2 failed!\n");
        	printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

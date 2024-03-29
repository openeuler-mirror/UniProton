/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_trylock()
 *   Upon failure, it shall return:
 *   -[EBUSY]   The pthread_mutex_trylock_4_1_mutex could not be acquired because it was already locked.

 * Steps: 
 *   -- Initilize a pthread_mutex_trylock_4_1_mutex object
 *   -- Lock the pthread_mutex_trylock_4_1_mutex using pthread_mutex_lock()
 *   -- Try to lock the pthread_mutex_trylock_4_1_mutex using pthread_mutex_trylock() and expect EBUSY
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_mutex_t    pthread_mutex_trylock_4_1_mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_trylock_4_1()
{
  	int           	rc;

	if((rc=pthread_mutex_lock(&pthread_mutex_trylock_4_1_mutex))!=0) {
		printf("Error at pthread_mutex_lock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	    		
   	rc = pthread_mutex_trylock(&pthread_mutex_trylock_4_1_mutex);
      	if(rc!=EBUSY) {
        	printf("Expected %d(EBUSY), got %d\n",EBUSY,rc);
        	printf("Test FAILED\n");
		return PTS_FAIL;
      	}
    	
    	pthread_mutex_unlock(&pthread_mutex_trylock_4_1_mutex);
  	pthread_mutex_destroy(&pthread_mutex_trylock_4_1_mutex);

	printf("Test PASSED\n");
	return PTS_PASS;
}

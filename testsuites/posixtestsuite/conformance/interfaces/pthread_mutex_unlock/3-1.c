/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_unlock()
 *   Upon succesful completion, it shall return zero
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int pthread_mutex_unlock_3_1()
{
	pthread_mutex_t  mutex;
	int rc;

	/* Initialize a mutex object */
	if((rc=pthread_mutex_init(&mutex,NULL)) != 0) {
		printf("Error at pthread_mutex_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	
	if((rc=pthread_mutex_lock(&mutex)) != 0) {
		printf("Error at pthread_mutex_lock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Release the mutex using pthread_mutex_unlock() */
	if((rc=pthread_mutex_unlock(&mutex)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	
	/* Check if returned values are tolerable */
	else if(rc == EPERM) {
		printf("Current thread does not own the mutex\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EINVAL) {
		printf("Invalid mutex object\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EAGAIN) {
		printf("The maximum number of recursive locks has been exceeded\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}

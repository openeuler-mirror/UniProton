/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_destroy()
 * 	It shall be safe to destroy an initialized pthread_mutex_destroy_5_1_mutex that is unlocked. 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

pthread_mutex_t  pthread_mutex_destroy_5_1_mutex;

int pthread_mutex_destroy_5_1()
{
	int rc;
	
	/* Initialize pthread_mutex_destroy_5_1_mutex with the default pthread_mutex_destroy_5_1_mutex attributes */
	if((rc=pthread_mutex_init(&pthread_mutex_destroy_5_1_mutex, NULL)) != 0) {
		printf("Fail to initialize pthread_mutex_destroy_5_1_mutex, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Lock pthread_mutex_destroy_5_1_mutex */
	if((rc=pthread_mutex_lock(&pthread_mutex_destroy_5_1_mutex)) != 0) {
		printf("Error at pthread_mutex_lock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	sleep(1);
	/* Unlock */
	if((rc=pthread_mutex_unlock(&pthread_mutex_destroy_5_1_mutex)) != 0) {
		printf("Error at pthread_mutex_unlock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	/* Destroy pthread_mutex_destroy_5_1_mutex after it is unlocked */
	if((rc=pthread_mutex_destroy(&pthread_mutex_destroy_5_1_mutex)) != 0) {
		printf("Fail to destroy pthread_mutex_destroy_5_1_mutex after being unlocked, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

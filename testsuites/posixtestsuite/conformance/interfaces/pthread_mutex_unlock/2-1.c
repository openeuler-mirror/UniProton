/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_unlock()
 *   If there are threads blocked on the pthread_mutex_unlock_2_1_mutex object referenced by 'pthread_mutex_unlock_2_1_mutex' when
 *   pthread_mutex_unlock() is called, resulting in the pthread_mutex_unlock_2_1_mutex becoming available,
 *   the scheduling policy shall determine which thread shall acquire the pthread_mutex_unlock_2_1_mutex.

 * NOTES: 
 *   The default scheduling policy is implementation dependent, thus this case 
 *   will only demo the scheduling sequence instead of testing it.
    
 * Steps: 
 *   -- Initialize a pthread_mutex_unlock_2_1_mutex to protect a global variable 'pthread_mutex_unlock_2_1_value'
 *   -- Create N threads. Each is looped M times to acquire the pthread_mutex_unlock_2_1_mutex, 
        increase the pthread_mutex_unlock_2_1_value, and then release the pthread_mutex_unlock_2_1_mutex.
 *   -- Check if the pthread_mutex_unlock_2_1_value has increased properly (M*N); a broken pthread_mutex_unlock_2_1_mutex 
        implementation may cause lost augments.
 *
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define    THREAD_NUM  	6
#define    LOOPS     	3

void *pthread_mutex_unlock_2_1_func(void *parm);

pthread_mutex_t    pthread_mutex_unlock_2_1_mutex = PTHREAD_MUTEX_INITIALIZER;
int                pthread_mutex_unlock_2_1_value;	/* pthread_mutex_unlock_2_1_value protected by pthread_mutex_unlock_2_1_mutex */

int pthread_mutex_unlock_2_1()
{
  	int                   i, rc;
  	pthread_t             threads[THREAD_NUM];

  	/* Create threads */
  	fprintf(stderr,"Creating %d threads\n", THREAD_NUM);
  	for (i=0; i<THREAD_NUM; ++i)
    		rc = pthread_create(&threads[i], NULL, pthread_mutex_unlock_2_1_func, NULL);

	/* Wait to join all threads */
  	for (i=0; i<THREAD_NUM; ++i)
    		pthread_join(threads[i], NULL);
  	pthread_mutex_destroy(&pthread_mutex_unlock_2_1_mutex);
  
  	/* Check if the final pthread_mutex_unlock_2_1_value is as expected */
  	if(pthread_mutex_unlock_2_1_value != (THREAD_NUM) * LOOPS) {
	  	fprintf(stderr,"Using %d threads and each loops %d times\n", THREAD_NUM, LOOPS);
    		fprintf(stderr,"Final pthread_mutex_unlock_2_1_value must be %d instead of %d\n", (THREAD_NUM)*LOOPS, pthread_mutex_unlock_2_1_value);
		return PTS_UNRESOLVED;
  	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}

void *pthread_mutex_unlock_2_1_func(void *parm)
{
  	int   i, tmp;
  	int   rc = 0;
  	pthread_t  self = pthread_self();

	/* Loopd M times to acquire the pthread_mutex_unlock_2_1_mutex, increase the pthread_mutex_unlock_2_1_value, 
	   and then release the pthread_mutex_unlock_2_1_mutex. */
	   
  	for (i=0; i<LOOPS; ++i) {
      		rc = pthread_mutex_lock(&pthread_mutex_unlock_2_1_mutex);
      		if(rc!=0) {
        		fprintf(stderr,"Error on pthread_mutex_lock(), rc=%d\n", rc);
			return (void*)(PTS_UNRESOLVED);
      		}

    		tmp = pthread_mutex_unlock_2_1_value;
    		tmp = tmp+1;
    		fprintf(stderr,"Thread(0x%p) holds the pthread_mutex_unlock_2_1_mutex\n",(void*)self);
    		nsleep(1000000);	  /* delay the increasement operation */
    		pthread_mutex_unlock_2_1_value = tmp;

      		rc = pthread_mutex_unlock(&pthread_mutex_unlock_2_1_mutex);
      		if(rc!=0) {
        		fprintf(stderr,"Error on pthread_mutex_unlock(), rc=%d\n", rc);
 			return (void*)(PTS_UNRESOLVED);
      		}
    		sleep(1);
  	}
  	pthread_exit(0);
  	return (void*)(0);
}

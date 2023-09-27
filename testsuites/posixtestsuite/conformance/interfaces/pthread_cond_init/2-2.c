/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.

 
 * This sample test aims to check the following assertion:
 *
 * If attr is NULL, the effect is the same as passing the address 
 * of a default condition variable attributes object.

 * The steps are:
 * -> Create two cond vars, one with NULL attribute and 
 *    the other with a default attribute.
 * -> Compare those two cond vars:
 *    -> If the Clock Selection option and the Timers option are supported, 
 *       do both condvars use the same clock?
 *       (steps to do the comparison:
 *          - test whether the system supports the CS option
 *          - get the clock_id of the default cond attr
 *          - wait for the two conditions with a timeout of 3 days (for exemple)
 *          - set_time the clock_id at a later time than the timeout.
 *          - if the set_time fails, consider the feature as passed (how to test?)
 *          - if the set_time succeeds, check whether both conds have the same behavior.
 *       )
 */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <unistd.h>

 #include <errno.h>
 #include <time.h> /* we need the clock_settime routine */
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void output_init()
  * void printf(char * string, ...)
  * 
  * Those may be used to printf information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

typedef struct
{
	pthread_mutex_t * pmtx;
	pthread_cond_t * pcnd;
	struct timespec * timeout;
	unsigned short ctrl;
	int rc;
} datatest_t;

#define FLAG_TIMEDOUT  (1 << 15) /* Will be set if the function exited with a timeout error */
#define FLAG_AWAKEN    (1 << 14) /* Will be set if the function exited with no error and the next flag was set */
#define FLAG_CONDWAKE  (1 << 13) /* the boolean associated with the condition */
#define FLAG_INWAIT    (1 << 12) /* Child thread got the mutex locked */
#define MASK_COUNTER  ((1 << 12) - 1) /* Those bits are reserved for a counter value */ 

static datatest_t dtN; /* Data structure for the Null attribute condvar */
static datatest_t dtI; /* Data structure for the Initializer condvar */

/****
 * test_timeout
 *  This function will receive a datatest_t argument. 
 *  It will lock the mutex, then mark the control value.
 *  and enter a cond timedwait with the argument timeout.
 */
void * pthread_cond_init_2_2_test_timeout(void * arg)
{
	int ret;
	unsigned short cnt = 0;
	datatest_t * dt = (datatest_t *)arg;
	
	/* lock the test mutex */
	ret = pthread_mutex_lock(dt->pmtx);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to lock mutex in test_timeout");  }
	
	/* Signal the parent thread that we are ready to wait */
	dt->ctrl |= FLAG_INWAIT;
	
	/* Enter the timed wait */
	do
	{
		dt->rc = pthread_cond_timedwait(dt->pcnd, dt->pmtx, dt->timeout);
		cnt = dt->ctrl & MASK_COUNTER;
		cnt++;
		cnt &= MASK_COUNTER;
		dt->ctrl &= ~MASK_COUNTER;
		dt->ctrl += cnt;
	} while ((dt->rc == 0) && ((dt->ctrl & FLAG_CONDWAKE) == 0));
	
	/* Set the flags */
	if (dt->rc == 0)
	{
		dt->ctrl |= FLAG_AWAKEN;
	}
	if (dt->rc == ETIMEDOUT)
	{
		dt->ctrl |= FLAG_TIMEDOUT;
	}
	
	/* Unlock the mutex and exit */
	ret = pthread_mutex_unlock(dt->pmtx);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to unlock the test mutex in test_timeout");  }
	
	return NULL;
}

/****
 * do_cs_test
 *  This function will take care of the clock testing.
 */
 
pthread_cond_t cndI = PTHREAD_COND_INITIALIZER; 

int pthread_cond_init_2_2_do_cs_test(void)
{
	int ret;
	int result=0;
	
	pthread_mutex_t mtxN, mtxI;
	pthread_cond_t cndN;
	
	pthread_condattr_t ca;
	
	pthread_t thN, thD;
	
	struct timespec ts, timeout;
	
	clockid_t  cid;

	/* The 3 next data aim to minimize the impact on the
	 *  system time, when the monotonic clock is supported
	 */
	long monotonic_clk;
	struct timespec diff;
	char sens=0;
	
	/* We are going to initialize the cond vars and the mutexes */
	dtN.pmtx = &mtxN;
	dtI.pmtx = &mtxI;
	dtN.pcnd = &cndN;
	dtI.pcnd = &cndI;
	ret = pthread_mutex_init(&mtxI, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize a default mutex");  }
	ret = pthread_mutex_init(&mtxN, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize a default mutex");  }
	ret = pthread_condattr_init(&ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize cond attribute object");  }

#if 0	/*  Test if the testcase is valid: change the clock from the "NULL" cond var */
	pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
	ret = pthread_cond_init(&cndN, &ca);
	pthread_condattr_setclock(&ca, CLOCK_REALTIME);
#else
	ret = pthread_cond_init(&cndN, NULL);
#endif
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to initialize the NULL attribute conditional variable.");  }
	
	dtI.ctrl = 0;
	dtN.ctrl = 0;
	dtI.rc   = 0;
	dtN.rc   = 0;
	
	#if VERBOSE > 1
	printf("Data initialized successfully for CS test.\n");
	#endif
	
	monotonic_clk = sysconf(_SC_MONOTONIC_CLOCK);
	#if VERBOSE > 1
	printf("Sysconf for monotonix clock: %li\n", monotonic_clk);
	#endif
	
	/* Get the default clock ID */
	ret = pthread_condattr_getclock(&ca, &cid);
	if (ret != 0)
	{  UNRESOLVED(ret, "Unable to get clockid from the default cond attribute object");  }
	
	/* Check whether we can set the system clock */
	
	/* Backup the current monotonic time if available */
	if (monotonic_clk != -1L)
	{
		ret = clock_gettime(CLOCK_MONOTONIC, &diff);
		if (ret != 0)
		{
			printf("Clock_gettime(CLOCK_MONOTONIC) failed when the system claims to support this option\n");
			monotonic_clk = -1L;
		}
	}
	ret = clock_gettime(cid, &ts);
	if (ret != 0)
	{  UNRESOLVED(errno, "Unable to get clock time");  }
	
	
	ret = clock_settime(cid, &ts);
	if (ret != 0)
	{
		#if VERBOSE > 1
		printf("clock_settime failed (%s)\n", strerror(errno));
		printf("We cannot test if both cond uses the same clock then...\n");
		#endif
		UNTESTED("Was unable to set the default clock time. Need more privileges?");
	}
	else /* We can do the test */
	{
		#if VERBOSE > 1
		printf("clock_settime succeeded\n");
		#endif
		
		if (monotonic_clk != -1L)
		{
			#if VERBOSE > 2
			printf("Monotonic clock : %10i.%09li\n", diff.tv_sec, diff.tv_nsec);
			printf("Default clock   : %10i.%09li\n", ts.tv_sec, ts.tv_nsec);
			#endif
			/* Save the decay between default clock and clock MONOTONIC */
			if (diff.tv_sec > ts.tv_sec)
			{
				sens = -1; /* monotonic was bigger than system */
				if (diff.tv_nsec < ts.tv_nsec)
				{
					diff.tv_nsec += 1000000000 - ts.tv_nsec;
					diff.tv_sec  -= 1 + ts.tv_sec;
				}
				else
				{
					diff.tv_nsec -= ts.tv_nsec;
					diff.tv_sec -= ts.tv_sec;
				}
			}
			else
			{
				if (diff.tv_sec == ts.tv_sec)
				{
					diff.tv_sec = 0;
					if (diff.tv_nsec > ts.tv_nsec)
					{
						sens = -1;
						diff.tv_nsec -= ts.tv_nsec;
					}
					else
					{
						sens = 1; /* Default clock was bigger than monotonic */
						diff.tv_nsec = ts.tv_nsec - diff.tv_nsec;
					}
				}
				else /* ts.tv_sec > diff.tv_sec */
				{
					sens = 1;
					if (ts.tv_nsec < diff.tv_nsec)
					{
						diff.tv_nsec = 1000000000 + ts.tv_nsec - diff.tv_nsec;
						diff.tv_sec = ts.tv_sec - (1 + diff.tv_sec);
					}
					else
					{
						diff.tv_nsec = ts.tv_nsec - diff.tv_nsec;
						diff.tv_sec = ts.tv_sec - diff.tv_sec;
					}
				}
			}
						
			#if VERBOSE > 2
			printf("Computed diff   : %10i.%09li\n", diff.tv_sec, diff.tv_nsec);
			printf("With monotonic %s than default\n", sens>0?"smaller":"bigger");
			#endif
		}
		
		/* Prepare the timeout parameters */
		timeout.tv_nsec = 0;
		timeout.tv_sec = ts.tv_sec + 260000; /* About 3 days later */
		dtN.timeout = &timeout;
		dtI.timeout = &timeout;
		
		/* create the threads */
		ret = pthread_create(&thD, NULL, pthread_cond_init_2_2_test_timeout, &dtI);
		if (ret != 0)
		{  UNRESOLVED(ret, "Unable to create a thread");  }
		ret = pthread_create(&thN, NULL, pthread_cond_init_2_2_test_timeout, &dtN);
		if (ret != 0)
		{  UNRESOLVED(ret, "Unable to create a thread");  }
		
		/* Lock the two mutex and make sure the threads are in wait */
		ret = pthread_mutex_lock(&mtxN);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		while ((dtN.ctrl & FLAG_INWAIT) == 0)
		{
			ret = pthread_mutex_unlock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			sched_yield();
			ret = pthread_mutex_lock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		}
		ret = pthread_mutex_lock(&mtxI);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		while ((dtI.ctrl & FLAG_INWAIT) == 0)
		{
			ret = pthread_mutex_unlock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			sched_yield();
			ret = pthread_mutex_lock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		}
		
		/* Now both threads are in the timed wait */
		#if VERBOSE > 1
		printf("Two threads are created and waiting.\n");
		printf("About to change the default clock value.\n");
		#endif
		
		/* We re-read the default clock time, to introduce minimal error on the clock */
		ret = clock_gettime(cid, &ts);
		if (ret != 0)  {  UNRESOLVED(errno, "Unable to get clock time");  }
		
		ts.tv_sec += 604800;  /* Exactly 1 week forth */
		
		/* We set the clock to a date after the timeout parameter */
		ret = clock_settime(cid, &ts);
		if (ret != 0)  {  UNRESOLVED(errno, "Unable to set clock time (again)");  }
		
		/* unlock the two mutex */
		ret = pthread_mutex_unlock(&mtxI);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
		ret = pthread_mutex_unlock(&mtxN);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
		
		/* Let the others threads run */
		sched_yield();
		
		#if VERBOSE > 1
		printf("Checking that both threads have timedout...\n");
		#endif
		
		/* Relock the mutexs */
		ret = pthread_mutex_lock(&mtxN);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		ret = pthread_mutex_lock(&mtxI);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		
		/* Check whether the thread has timedout */
		if (dtI.rc == 0)
		{
			#if VERBOSE > 0
			printf("The thread was not woken when the clock was changed so as the timeout expired\n");
			printf("Going to simulate this POSIX behavior...\n");
			#endif
			ret = pthread_cond_signal(&cndN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to signal the Null attribute condition");  }
			ret = pthread_cond_signal(&cndI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to signal the Default attribute condition");  }
			
			/* The threads will now report a spurious wake up ... */
			
			/* We give the threads another chance to timeout */
			/* unlock the two mutex */
			ret = pthread_mutex_unlock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			ret = pthread_mutex_unlock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			
			/* Let the others threads run */
			sched_yield();
			
			#if VERBOSE > 1
			printf("Rechecking that both threads have timedout...\n");
			#endif
			
			/* Relock the mutexs */
			ret = pthread_mutex_lock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
			ret = pthread_mutex_lock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to lock a mutex");  }
		}
			
		
		/* Process the Null condvar */
		if ((dtN.ctrl & FLAG_TIMEDOUT) != 0)
		{
			#if VERBOSE > 1
			printf("Null attribute cond var timed out\n");
			#endif
			/* Join the thread */
			ret = pthread_join(thN, NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "Join thread failed");  }
			/* Unlock the mutex */
			ret = pthread_mutex_unlock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
		}
		else
		{
			#if VERBOSE > 1
			printf("Null attribute cond var did not time out\n");
			#endif
			/* Signal the condition */
			dtN.ctrl |= FLAG_CONDWAKE;
			ret = pthread_cond_signal(&cndN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to signal the Null attribute condition");  }
			/* Unlock the mutex and join the thread */
			ret = pthread_mutex_unlock(&mtxN);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			/* Join the thread */
			ret = pthread_join(thN, NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "Join thread failed");  }
		}
		
		/* Process the Default condvar */
		if ((dtI.ctrl & FLAG_TIMEDOUT) != 0)
		{
			#if VERBOSE > 1
			printf("Default attribute cond var timed out\n");
			#endif
			/* Join the thread */
			ret = pthread_join(thD, NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "Join thread failed");  }
			/* Unlock the mutex */
			ret = pthread_mutex_unlock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
		}
		else
		{
			#if VERBOSE > 1
			printf("Default attribute cond var did not time out\n");
			#endif
			/* Signal the condition */
			dtI.ctrl |= FLAG_CONDWAKE;
			ret = pthread_cond_signal(&cndI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to signal the Default attribute condition");  }
			/* Unlock the mutex and join the thread */
			ret = pthread_mutex_unlock(&mtxI);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to unlock a mutex");  }
			/* Join the thread */
			ret = pthread_join(thD, NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "Join thread failed");  }
		}
		
		/* Set the system time back to its value */
		if (monotonic_clk == -1L) /* Monotonic is not supported */
		{
			ret = clock_gettime(cid, &ts);
			if (ret != 0)  {  UNRESOLVED(errno, "Unable to get clock time");  }
			
			ts.tv_sec -= 604800;  /* Exactly 1 week back */ 
			
			/* We set the clock to a date after the timeout parameter */
			ret = clock_settime(cid, &ts);
			if (ret != 0)  {  UNRESOLVED(errno, "Unable to set clock time (3rd time)");  }
			#if VERBOSE > 1
			printf("Default clock was set back\n");
			#endif
		}
		else
		{
			/* Read the monotonic time */
			ret = clock_gettime(CLOCK_MONOTONIC, &ts);
			if (ret != 0)  {  UNRESOLVED(errno, "Unable to get monotonic clock time");  }
			
			/* Apply the difference back */
			if (sens > 0) /* monotonic was smaller than system => we must add the diff value */
			{
				ts.tv_sec += diff.tv_sec;
				ts.tv_nsec += diff.tv_nsec;
				if (ts.tv_nsec >= 1000000000)
				{
					ts.tv_nsec -= 1000000000;
					ts.tv_sec += 1;
				}
			}
			else /* monotonic was bigger than system => we must remove the diff value */
			{
				ts.tv_sec -= diff.tv_sec;
				if (ts.tv_nsec < diff.tv_nsec)
				{
					ts.tv_sec -= 1;
					ts.tv_nsec += 1000000000 - diff.tv_nsec;
				}
				else
				{
					ts.tv_nsec -= diff.tv_nsec;
				}
			}
			
			/* We set the clock to a date after the timeout parameter */
			ret = clock_settime(cid, &ts);
			if (ret != 0)  {  UNRESOLVED(errno, "Unable to set clock time (3rd time)");  }
			
			#if VERBOSE > 1
			printf("Default clock was set back using monotonic clock as a reference\n");
			#endif
		}
		
		/* Compare the two cond vars */
		#if VERBOSE > 2
		printf("Default attribute cond var timedwait return value: %d\n", dtI.rc);
		printf("Default attribute cond var exited with a timeout : %s\n", (dtI.ctrl & FLAG_TIMEDOUT)?"yes":"no");
		printf("Default attribute cond var had to be signaled    : %s\n", (dtI.ctrl & FLAG_CONDWAKE)?"yes":"no");
		printf("Default attribute cond var exited as signaled    : %s\n", (dtI.ctrl & FLAG_AWAKEN)?"yes":"no");
		printf("Default attribute cond var spurious wakeups      : %d\n", (dtI.ctrl & MASK_COUNTER) - 1);
		printf(" Null   attribute cond var timedwait return value: %d\n", dtN.rc);
		printf(" Null   attribute cond var exited with a timeout : %s\n", (dtN.ctrl & FLAG_TIMEDOUT)?"yes":"no");
		printf(" Null   attribute cond var had to be signaled    : %s\n", (dtN.ctrl & FLAG_CONDWAKE)?"yes":"no");
		printf(" Null   attribute cond var exited as signaled    : %s\n", (dtN.ctrl & FLAG_AWAKEN)?"yes":"no");
		printf(" Null   attribute cond var spurious wakeups      : %d\n", (dtN.ctrl & MASK_COUNTER) - 1);
		#endif
		if ((dtN.ctrl == dtI.ctrl) && (dtN.rc == dtI.rc))
		{
			result = 0;
			#if VERBOSE > 1
			printf("The two cond vars behaved exactly in the same maneer\n");
			#endif
		}
		else
		{
			if ((dtN.ctrl & FLAG_TIMEDOUT) != (dtI.ctrl & FLAG_TIMEDOUT))
			{
				result += 1; /* The test has failed */
			}
			else
			{
				if (dtN.rc != dtI.rc)
				{
					printf("Error codes were different: N:%d D:%d\n",dtN.rc, dtI.rc);
					UNRESOLVED(dtN.rc>dtI.rc?dtN.rc:dtI.rc, "Different error codes?");
				}
				if ((dtN.ctrl & FLAG_AWAKEN) == (dtI.ctrl & FLAG_AWAKEN))
				{
					/* The number of spurious wakeups is different */
					printf("Different spurious wakeups: N:%d D:%d\n", 
						(dtN.ctrl & MASK_COUNTER) - 1, 
						(dtI.ctrl & MASK_COUNTER) - 1);
					
					result = 0; /* We don't consider this as a fail case */
				}
			}
		}
	}
	
	/* We can cleanup things now */
	ret = pthread_cond_destroy(&cndN);
	if (ret != 0)
	{  UNRESOLVED(ret, "Cond destroy failed");  }
	ret = pthread_condattr_destroy(&ca);
	if (ret != 0)
	{  UNRESOLVED(ret, "Cond attr destroy failed");  }
	ret = pthread_mutex_destroy(&mtxN);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex destroy failed");  }
	ret = pthread_mutex_destroy(&mtxI);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex destroy failed");  }
	
	return result;
}

/****
 * Main function
 */
int pthread_cond_init_2_2(int argc, char * argv[])
{
	long opt_TMR, opt_CS;
	int ret=0;

	output_init();
	
	#if VERBOSE > 1
	printf("Test starting...\n");
	#endif

	opt_TMR=sysconf(_SC_TIMERS);
	opt_CS =sysconf(_SC_CLOCK_SELECTION);
	
	#if VERBOSE > 1
	printf("Timers option : %li\n", opt_TMR);
	printf("Clock Selection option : %li\n", opt_CS);
	#endif
	printf("-----pthread_cond_init_2_2 opt_TMR =%d, opt_CS=%d ---\n ", opt_TMR, opt_CS);
	if ((opt_TMR != -1L) && (opt_CS != -1L))
	{
		#if VERBOSE > 0
		printf("Starting clock test\n");
		#endif
		ret = pthread_cond_init_2_2_do_cs_test();
	}
	else
	{
		UNTESTED("This test requires unsupported features");
	}		
	if (ret != 0)
	{  FAILED("The cond vars use different clocks.");  }
	
	PASSED
}

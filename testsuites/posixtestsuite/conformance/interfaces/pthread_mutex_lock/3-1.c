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
 *
 
 
 * This sample test aims to check the following assertion:
 * The function does not return an error code of EINTR 


 * The steps are:
 * 
 * -> Create a thread which loops on pthread_mutex_lock and pthread_mutex_unlock
 *      operations.
 * -> Create another thread which loops on sending a signal to the first thread.
 * 
 * 
 */

 /* 
  * - adam.li@intel.com 2004-05-13
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/ 
  *   for general information
  */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We enable the following line to have mutex attributes defined */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
 
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <signal.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <stdarg.h>
 
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
  * void pthread_mutex_lock_output_init()
  * void printf(char * string, ...)
  * 
  * Those may be used to printf information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef __EMSCRIPTEN__
// See https://github.com/emscripten-core/emscripten/issues/14892
#define WITH_SYNCHRO
#endif
#ifndef VERBOSE
#define VERBOSE 1 
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char pthread_mutex_lock_3_1_do_it=1;
unsigned long pthread_mutex_lock_3_1_count_ope=0;
pthread_mutex_t pthread_mutex_lock_3_1_count_protect = PTHREAD_MUTEX_INITIALIZER;
#ifdef WITH_SYNCHRO
sem_t pthread_mutex_lock_3_1_semsig1;
sem_t pthread_mutex_lock_3_1_semsig2;
unsigned long pthread_mutex_lock_3_1_count_sig=0;
#endif
sem_t pthread_mutex_lock_3_1_semsync;

typedef struct 
{
	pthread_t   	*thr;
	int	sig;
#ifdef WITH_SYNCHRO
	sem_t	*sem;
#endif
} pthread_mutex_lock_3_1_thestruct;

/* the following function keeps on sending the signal to the thread pointed by arg
 *  If WITH_SYNCHRO is defined, the target thread has a handler for the signal */
void * pthread_mutex_lock_3_1_sendsig (void * arg)
{
	pthread_mutex_lock_3_1_thestruct *thearg = (pthread_mutex_lock_3_1_thestruct *) arg;
	int ret;
	while (pthread_mutex_lock_3_1_do_it)
	{
		#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem)))
		{ UNRESOLVED(errno, "Sem_wait in pthread_mutex_lock_3_1_sendsig"); }
		pthread_mutex_lock_3_1_count_sig++;
		#endif

		if ((ret = pthread_kill (*(thearg->thr), thearg->sig)))
		{ UNRESOLVED(ret, "Pthread_kill in pthread_mutex_lock_3_1_sendsig"); }
		
	}
	
	return NULL;
}

/* Next are the signal handlers. */
void pthread_mutex_lock_3_1_sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if ((sem_post(&pthread_mutex_lock_3_1_semsig1)))
	{ UNRESOLVED(errno, "Sem_post in signal handler 1"); }
#endif
}
void pthread_mutex_lock_3_1_sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if ((sem_post(&pthread_mutex_lock_3_1_semsig2)))
	{ UNRESOLVED(errno, "Sem_post in signal handler 2"); }
#endif	
}

/* The following function loops on init/destroy some mutex (with different attributes)
 * it does check that no error code of EINTR is returned */

void * pthread_mutex_lock_3_1_threaded(void * arg)
{
	pthread_mutexattr_t ma[4], *pma[5];
	pthread_mutex_t m[5];
	int i;
	int ret;
	
	/* We need to register the signal handlers */
	struct sigaction sa;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = pthread_mutex_lock_3_1_sighdl1;
	if ((ret = sigaction (SIGUSR1, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler1"); }
	sa.sa_handler = pthread_mutex_lock_3_1_sighdl2;
	if ((ret = sigaction (SIGUSR2, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler2"); }
 	
	/* Initialize the different mutex */
	pma[4]=NULL;
	
	for (i=0; i<4; i++)
	{
		pma[i]=&ma[i];
		if ((ret = pthread_mutexattr_init(pma[i])))
		{ UNRESOLVED(ret, "pthread_mutexattr_init"); }
	}
	
	#ifndef WITHOUT_XOPEN
		if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_NORMAL)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (normal)"); }
		if ((ret = pthread_mutexattr_settype(pma[1], PTHREAD_MUTEX_ERRORCHECK)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (errorcheck)"); }
		if ((ret = pthread_mutexattr_settype(pma[2], PTHREAD_MUTEX_RECURSIVE)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (recursive)"); }
		if ((ret = pthread_mutexattr_settype(pma[3], PTHREAD_MUTEX_DEFAULT)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (default)"); }
		#if VERBOSE >1
		printf("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT initialized\n");
		#endif
	#else
		#if VERBOSE > 0
		printf("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT unavailable\n");
		#endif
	#endif

	for (i=0; i<5; i++)
	{
		ret = pthread_mutex_init(&m[i], pma[i]);
		if (ret == EINTR)
		{
			FAILED("pthread_mutex_init returned EINTR");
		}
		if (ret != 0)
		{
			UNRESOLVED(ret, "pthread_mutex_init failed");
		}
	}
	/* The mutex are ready, we will loop on lock/unlock now */
		
	while (pthread_mutex_lock_3_1_do_it)
	{
		for (i=0; i<5; i++)
		{
			ret = pthread_mutex_lock(&m[i]);
			if (ret == EINTR)
			{
				FAILED("pthread_mutex_lock returned EINTR");
			}
			if (ret != 0)
			{
				UNRESOLVED(ret, "pthread_mutex_lock failed");
			}
			ret = pthread_mutex_unlock(&m[i]);
			if (ret == EINTR)
			{
				FAILED("pthread_mutex_unlock returned EINTR");
			}
			if (ret != 0)
			{
				UNRESOLVED(ret, "pthread_mutex_unlock failed");
			}
		}
		ret = pthread_mutex_lock(&pthread_mutex_lock_3_1_count_protect);
		if (ret == EINTR)
		{
			FAILED("pthread_mutex_lock returned EINTR");
		}
		if (ret != 0)
		{
			UNRESOLVED(ret, "pthread_mutex_lock failed");
		}
		pthread_mutex_lock_3_1_count_ope++;
		pthread_mutex_unlock(&pthread_mutex_lock_3_1_count_protect);
		if (ret == EINTR)
		{
			FAILED("pthread_mutex_unlock returned EINTR");
		}
		if (ret != 0)
		{
			UNRESOLVED(ret, "pthread_mutex_unlock failed");
		}
	}

	/* Now we can destroy the mutex objects */
	for (i=0; i<4; i++)
	{
		if ((ret = pthread_mutexattr_destroy(pma[i])))
		{ UNRESOLVED(ret, "pthread_mutexattr_init"); }
	}
	for (i=0; i<5; i++)
	{
		ret = pthread_mutex_destroy(&m[i]);
		if (ret == EINTR)
		{
			FAILED("pthread_mutex_destroy returned EINTR");
		}
		if (ret != 0)
		{
			UNRESOLVED(ret, "pthread_mutex_destroy failed");
		}
	}
	
	do {
		ret = sem_wait(&pthread_mutex_lock_3_1_semsync);
	} while (ret && (errno ==  EINTR));
	if (ret)
	{ UNRESOLVED(errno, "Could not wait for sig senders termination"); }
	
	return NULL;
}

/* At last (but not least) we need a main */
int pthread_mutex_lock_3_1 (int argc, char * argv[])
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	pthread_mutex_lock_3_1_thestruct arg1, arg2;
	
	pthread_mutex_lock_output_init();

	#ifdef WITH_SYNCHRO
	#if VERBOSE >1
	printf("Running in synchronized mode\n");
	#endif
	if ((sem_init(&pthread_mutex_lock_3_1_semsig1, 0, 1)))
	{ UNRESOLVED(errno, "Semsig1  init"); }
	if ((sem_init(&pthread_mutex_lock_3_1_semsig2, 0, 1)))
	{ UNRESOLVED(errno, "Semsig2  init"); }
	#endif
	
	if ((sem_init(&pthread_mutex_lock_3_1_semsync, 0, 0)))
	{ UNRESOLVED(errno, "pthread_mutex_lock_3_1_semsync init"); }

	#if VERBOSE >1
	printf("Starting the worker thread\n");
	#endif
	if ((ret = pthread_create(&th_work, NULL, pthread_mutex_lock_3_1_threaded, NULL)))
	{ UNRESOLVED(ret, "Worker thread creation failed"); }
	
	arg1.thr = &th_work;
	arg2.thr = &th_work;
	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &pthread_mutex_lock_3_1_semsig1;
	arg2.sem = &pthread_mutex_lock_3_1_semsig2;
#endif
	
	#if VERBOSE >1
	printf("Starting the signal sources\n");
	#endif
	if ((ret = pthread_create(&th_sig1, NULL, pthread_mutex_lock_3_1_sendsig, (void *)&arg1)))
	{ UNRESOLVED(ret, "Signal 1 sender thread creation failed"); }
	if ((ret = pthread_create(&th_sig2, NULL, pthread_mutex_lock_3_1_sendsig, (void *)&arg2)))
	{ UNRESOLVED(ret, "Signal 2 sender thread creation failed"); }
	
	/* Let's wait for a while now */
	#if VERBOSE >1
	printf("Let the worker be killed for a second\n");
	#endif
	sleep(1);
	
	/* Now stop the threads and join them */
	#if VERBOSE >1
	printf("Stop everybody\n");
	#endif
	do { pthread_mutex_lock_3_1_do_it=0; }
	while (pthread_mutex_lock_3_1_do_it);
	
	if ((ret = pthread_join(th_sig1, NULL)))
	{ UNRESOLVED(ret, "Signal 1 sender thread join failed"); }
	if ((ret = pthread_join(th_sig2, NULL)))
	{ UNRESOLVED(ret, "Signal 2 sender thread join failed"); }

	#if VERBOSE >1
	printf("Signal sources are stopped, we can stop the worker\n");
	#endif
	if ((sem_post(&pthread_mutex_lock_3_1_semsync)))
	{ UNRESOLVED(errno, "could not post pthread_mutex_lock_3_1_semsync"); }
	
	if ((ret = pthread_join(th_work, NULL)))
	{ UNRESOLVED(ret, "Worker thread join failed"); }

	#if VERBOSE > 0
	printf("Test executed successfully.\n");
	printf("  %d mutex lock and unlock were done.\n", pthread_mutex_lock_3_1_count_ope);
	#ifdef WITH_SYNCHRO
	printf("  %d signals were sent meanwhile.\n", pthread_mutex_lock_3_1_count_sig);
	#endif 
	#endif	
	PASSED;
}

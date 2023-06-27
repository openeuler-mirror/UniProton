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

 
 * This sample pthread_detach_4_3_test aims to check the following assertion:
 *
 * The function does not return EINTR
 
 * The steps are:
 * -> kill a thread which calls pthread_detach()
 * -> check that EINTR is never returned
 
 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <string.h>
 #include <unistd.h>

 #include <sched.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <assert.h>
 #include <sys/wait.h>
 #include <time.h>
 #include <signal.h>
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the pthread_detach_4_3_test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_detach_output_init()
  * void pthread_detach_output(char * string, ...)
  * 
  * Those may be used to pthread_detach_output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#ifndef __EMSCRIPTEN__
// See https://github.com/emscripten-core/emscripten/issues/14892
#define WITH_SYNCHRO
#endif


/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * pthread_detach_scenarii: array of struct __pthread_detach_scenario type.
 * NSCENAR : macro giving the total # of pthread_detach_scenarii
 * pthread_detach_scenar_init(): function to call before use the pthread_detach_scenarii array.
 * pthread_detach_scenar_fini(): function to call after end of use of the pthread_detach_scenarii array.
 */


/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

char pthread_detach_4_3_do_it=1;
unsigned long pthread_detach_4_3_count_ope=0;
#ifdef WITH_SYNCHRO
sem_t pthread_detach_4_3_semsig1;
sem_t pthread_detach_4_3_semsig2;
unsigned long pthread_detach_4_3_count_sig=0;
#endif

sigset_t pthread_detach_4_3_usersigs;

typedef struct 
{
	int	sig;
#ifdef WITH_SYNCHRO
	sem_t	*sem;
#endif
} pthread_detach_4_3_thestruct;

/* the following function keeps on sending the signal to the process */
void * pthread_detach_4_3_sendsig (void * arg)
{
	pthread_detach_4_3_thestruct *thearg = (pthread_detach_4_3_thestruct *) arg;
	int ret;
	pid_t process;
	
	process=getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &pthread_detach_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 in signal thread");  }

	while (pthread_detach_4_3_do_it)
	{
		#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem)))
		{ UNRESOLVED(errno, "Sem_wait in pthread_detach_4_3_sendsig"); }
		pthread_detach_4_3_count_sig++;
		#endif

		ret = kill(process, thearg->sig);
		if (ret != 0)  { UNRESOLVED(errno, "Kill in pthread_detach_4_3_sendsig"); }
		
	}
	return NULL;
}

/* Next are the signal handlers. */
/* This one is registered for signal SIGUSR1 */
void pthread_detach_4_3_sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&pthread_detach_4_3_semsig1))
	{ UNRESOLVED(errno, "Sem_post in signal handler 1"); }
#endif
}
/* This one is registered for signal SIGUSR2 */
void pthread_detach_4_3_sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&pthread_detach_4_3_semsig2))
	{ UNRESOLVED(errno, "Sem_post in signal handler 2"); }
#endif	
}

/* Thread function -- almost does nothing */
void * pthread_detach_4_3_threaded(void * arg)
{
	int ret;
	
 	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &pthread_detach_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to unblock SIGUSR1 and SIGUSR2 in worker thread");  }

	ret = pthread_detach(pthread_self());
	if (ret == EINTR)  {  FAILED("pthread_detach() returned EINTR");  }
	
	/* Signal we're done */
	do { ret = sem_post(&pthread_detach_scenarii[pthread_detach_sc].sem); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to wait for the semaphore");  }
	
	/* return */
	return arg;
}

/* Test function -- creates the thread. */
void * pthread_detach_4_3_test(void * arg)
{
	int ret=0;
	pthread_t child;
	
 	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &pthread_detach_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 in signal thread");  }

	pthread_detach_sc = 0;
	
	while (pthread_detach_4_3_do_it)
	{
		#if VERBOSE > 5
		pthread_detach_output("-----\n");
		pthread_detach_output("Starting pthread_detach_4_3_test with scenario (%i): %s\n", pthread_detach_sc, pthread_detach_scenarii[pthread_detach_sc].descr);
		#endif
		
		pthread_detach_4_3_count_ope++;
		
		ret = pthread_create(&child, &pthread_detach_scenarii[pthread_detach_sc].ta, pthread_detach_4_3_threaded, NULL);
		switch (pthread_detach_scenarii[pthread_detach_sc].result)
		{
			case 0: /* Operation was expected to succeed */
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to create this thread");  }
				break;
			
			case 1: /* Operation was expected to fail */
				if (ret == 0)  {  UNRESOLVED(-1, "An error was expected but the thread creation succeeded");  }
				break;
			
			case 2: /* We did not know the expected result */
			default:
				#if VERBOSE > 5
				if (ret == 0)
					{ pthread_detach_output("Thread has been created successfully for this scenario\n"); }
				else
					{ pthread_detach_output("Thread creation failed with the error: %s\n", strerror(ret)); }
				#endif
				;
		}
		if (ret == 0) /* The new thread is running */
		{
			/* Just wait for the thread to terminate */
			do { ret = sem_wait(&pthread_detach_scenarii[pthread_detach_sc].sem); }
			while ((ret == -1) && (errno == EINTR));
			if (ret == -1)  {  UNRESOLVED(errno, "Failed to wait for the semaphore");  }
		}
		
		/* Change thread attribute for the next loop */
		pthread_detach_sc++;
		pthread_detach_sc %= NSCENAR;
	}
	return NULL;
}

/* Main function */
int pthread_detach_4_3 (int argc, char * argv[])
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	pthread_detach_4_3_thestruct arg1, arg2;
	struct sigaction sa;

	/* Initialize pthread_detach_output routine */
	pthread_detach_output_init();
	
	/* Initialize thread attribute objects */
	pthread_detach_scenar_init();
	
	/* We need to register the signal handlers for the PROCESS */
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = pthread_detach_4_3_sighdl1;
	if ((ret = sigaction (SIGUSR1, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler1"); }
	sa.sa_handler = pthread_detach_4_3_sighdl2;
	if ((ret = sigaction (SIGUSR2, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler2"); }

	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset(&pthread_detach_4_3_usersigs);
	ret = sigaddset(&pthread_detach_4_3_usersigs, SIGUSR1);
	ret |= sigaddset(&pthread_detach_4_3_usersigs, SIGUSR2);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to add SIGUSR1 or 2 to a signal set");  }
	
	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &pthread_detach_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 in main thread");  }

	#ifdef WITH_SYNCHRO
	if (sem_init(&pthread_detach_4_3_semsig1, 0, 1))
	{ UNRESOLVED(errno, "Semsig1  init"); }
	if (sem_init(&pthread_detach_4_3_semsig2, 0, 1))
	{ UNRESOLVED(errno, "Semsig2  init"); }
	#endif

	if ((ret = pthread_create(&th_work, NULL, pthread_detach_4_3_test, NULL)))
	{ UNRESOLVED(ret, "Worker thread creation failed"); }
	
	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &pthread_detach_4_3_semsig1;
	arg2.sem = &pthread_detach_4_3_semsig2;
#endif


	
	if ((ret = pthread_create(&th_sig1, NULL, pthread_detach_4_3_sendsig, (void *)&arg1)))
	{ UNRESOLVED(ret, "Signal 1 sender thread creation failed"); }
	if ((ret = pthread_create(&th_sig2, NULL, pthread_detach_4_3_sendsig, (void *)&arg2)))
	{ UNRESOLVED(ret, "Signal 2 sender thread creation failed"); }



	/* Let's wait for a while now */
	sleep(1);
	

	/* Now stop the threads and join them */
	do { pthread_detach_4_3_do_it=0; }
	while (pthread_detach_4_3_do_it);

	
	if ((ret = pthread_join(th_sig1, NULL)))
	{ UNRESOLVED(ret, "Signal 1 sender thread join failed"); }

	if ((ret = pthread_join(th_sig2, NULL)))
	{ UNRESOLVED(ret, "Signal 2 sender thread join failed"); }

	
	if ((ret = pthread_join(th_work, NULL)))
	{ UNRESOLVED(ret, "Worker thread join failed"); }


	pthread_detach_scenar_fini();

	#if VERBOSE > 0
	pthread_detach_output("Test executed successfully.\n");
	pthread_detach_output("  %d thread detached.\n", pthread_detach_4_3_count_ope);
	#ifdef WITH_SYNCHRO
	pthread_detach_output("  %d signals were sent meanwhile.\n", pthread_detach_4_3_count_sig);
	#endif 
	#endif

	printf("Test PASS\n");
	PASSED;
}


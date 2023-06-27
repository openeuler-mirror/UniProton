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

 
 * This sample pthread_mutex_trylock_4_3_test aims to check the following assertion:
 *
 * The function does not return EINTR
 
 * The steps are:
 * -> A thread is killed several times while calling pthread_mutex_trylock
 * -> check that EINTR is never returned
 
 */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We need the XSI extention for the mutex attributes
   and the mkstemp() routine */
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
 #include <unistd.h>

 #include <errno.h>
 #include <semaphore.h>
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
  *    where descr is a short text saying why the pthread_mutex_trylock_4_3_test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_mutex_trylock_output_init()
  * void pthread_mutex_trylock_output(char * string, ...)
  * 
  * Those may be used to pthread_mutex_trylock_output information.
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
/***********************************    Test case   *****************************************/
/********************************************************************************************/
struct _pthread_mutex_trylock_4_3_scenar
{
	int m_type; /* Mutex type to use */
	int m_pshared; /* 0: mutex is process-private (default) ~ !0: mutex is process-shared, if supported */
	char * descr; /* Case description */
}
pthread_mutex_trylock_4_3_scenarii[] =
{
	 {PTHREAD_MUTEX_DEFAULT,    0, "Default mutex"}
#ifndef WITHOUT_XOPEN
	,{PTHREAD_MUTEX_NORMAL,     0, "Normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 0, "Errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  0, "Recursive mutex"}
#endif

	,{PTHREAD_MUTEX_DEFAULT,    1, "Pshared mutex"}
#ifndef WITHOUT_XOPEN
	,{PTHREAD_MUTEX_NORMAL,     1, "Pshared Normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, "Pshared Errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, "Pshared Recursive mutex"}
#endif
};
#define NSCENAR (sizeof(pthread_mutex_trylock_4_3_scenarii)/sizeof(pthread_mutex_trylock_4_3_scenarii[0]))


char pthread_mutex_trylock_4_3_do_it=1;
char pthread_mutex_trylock_4_3_woken=0;
unsigned long pthread_mutex_trylock_4_3_count_ope=0;
#ifdef WITH_SYNCHRO
sem_t pthread_mutex_trylock_4_3_count_semsig1;
sem_t pthread_mutex_trylock_4_3_count_semsig2;
unsigned long pthread_mutex_trylock_4_3_count_sig=0;
#endif

sigset_t pthread_mutex_trylock_4_3_usersigs;

typedef struct 
{
	int	sig;
#ifdef WITH_SYNCHRO
	sem_t	*sem;
#endif
} pthread_mutex_trylock_4_3_thestruct;

/* the following function keeps on sending the signal to the process */
void * pthread_mutex_trylock_4_3_sendsig (void * arg)
{
	pthread_mutex_trylock_4_3_thestruct *thearg = (pthread_mutex_trylock_4_3_thestruct *) arg;
	int ret;
	pid_t process;
	
	process=getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &pthread_mutex_trylock_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 in signal thread");  }

	while (pthread_mutex_trylock_4_3_do_it)
	{
		#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem)))
		{ UNRESOLVED(errno, "Sem_wait in pthread_mutex_trylock_4_3_sendsig"); }
		pthread_mutex_trylock_4_3_count_sig++;
		#endif

		ret = kill(process, thearg->sig);
		if (ret != 0)  { UNRESOLVED(errno, "Kill in pthread_mutex_trylock_4_3_sendsig"); }
		
	}
	return NULL;
}

/* Next are the signal handlers. */
/* This one is registered for signal SIGUSR1 */
void pthread_mutex_trylock_4_3_sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&pthread_mutex_trylock_4_3_count_semsig1))
	{ UNRESOLVED(errno, "Sem_post in signal handler 1"); }
#endif
}
/* This one is registered for signal SIGUSR2 */
void pthread_mutex_trylock_4_3_sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&pthread_mutex_trylock_4_3_count_semsig2))
	{ UNRESOLVED(errno, "Sem_post in signal handler 2"); }
#endif	
}


/* Test function -- This one calls pthread_mutex_trylock and check that no EINTR is returned. */
void * pthread_mutex_trylock_4_3_test(void * arg)
{
	int ret=0;
	int i;
	long pshared;
	pthread_mutex_t mtx[NSCENAR+2];
	pthread_mutexattr_t ma[NSCENAR+1];
	
 	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &pthread_mutex_trylock_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to unblock SIGUSR1 and SIGUSR2 in worker thread");  }
	
	/* System abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	
	/* Initialize the mutex objects according to the pthread_mutex_trylock_4_3_scenarii */
	for (i=0; i<NSCENAR; i++)
	{
		ret = pthread_mutexattr_init(&ma[i]);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to initialize the mutex attribute object");  }
		#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma[i], pthread_mutex_trylock_4_3_scenarii[i].m_type);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set mutex type");  }
		#endif
		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (pthread_mutex_trylock_4_3_scenarii[i].m_pshared != 0))
		{
			ret = pthread_mutexattr_setpshared(&ma[i], PTHREAD_PROCESS_SHARED);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set the mutex process-shared");  }
		}

		/* Initialize the mutex */
		ret = pthread_mutex_init(&mtx[i], &ma[i]);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Mutex init failed");  }
		
	}
	/* Default mutexattr object */
	ret = pthread_mutexattr_init(&ma[i]);
	if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to initialize the mutex attribute object");  }
	ret = pthread_mutex_init(&mtx[i], &ma[i]);
	if (ret != 0)  {  UNRESOLVED(ret, "[parent] Mutex init failed");  }
	/* Default mutex */
	ret = pthread_mutex_init(&mtx[i+1], NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "[parent] Mutex init failed");  }
	
	
	/* do the real testing */
	while (pthread_mutex_trylock_4_3_do_it)
	{
		pthread_mutex_trylock_4_3_count_ope++;
		
		ret = pthread_mutex_trylock(&mtx[pthread_mutex_trylock_4_3_count_ope % (NSCENAR+2)]);
		if (ret == EINTR)  {  FAILED("EINTR was returned");  }
		if (ret != 0)  {  UNRESOLVED(ret, "1st trylock failed");  }
		
		ret = pthread_mutex_trylock(&mtx[pthread_mutex_trylock_4_3_count_ope % (NSCENAR+2)]);
		if (ret == EINTR)  {  FAILED("EINTR was returned");  }
		if (ret == 0)
		{
			ret = pthread_mutex_unlock(&mtx[pthread_mutex_trylock_4_3_count_ope % (NSCENAR+2)]);
			if (ret != 0)  {  UNRESOLVED(ret, "Unlocking the mutex failed");  }
			ret = EBUSY;
		}
		if (ret != EBUSY)  {  UNRESOLVED(ret, "Unexpected error was returned.");  }
		
		ret = pthread_mutex_unlock(&mtx[pthread_mutex_trylock_4_3_count_ope % (NSCENAR+2)]);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to unlock the mutex");  }
	}
	
	/* Destroy everything */
	for (i=0; i <= NSCENAR; i++)
	{
		ret = pthread_mutex_destroy(&mtx[i]);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex");  }
		
		ret = pthread_mutexattr_destroy(&ma[i]);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex attr object");  }
	}
	ret = pthread_mutex_destroy(&mtx[i]);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex");  }
	
	return NULL;
}



/* Main function */
int pthread_mutex_trylock_4_3 (int argc, char * argv[])
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	pthread_mutex_trylock_4_3_thestruct arg1, arg2;
	struct sigaction sa;
	
	/* Initialize pthread_mutex_trylock_output routine */
	pthread_mutex_trylock_output_init();
	
	/* We need to register the signal handlers for the PROCESS */
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = pthread_mutex_trylock_4_3_sighdl1;
	if ((ret = sigaction (SIGUSR1, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler1"); }
	sa.sa_handler = pthread_mutex_trylock_4_3_sighdl2;
	if ((ret = sigaction (SIGUSR2, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler2"); }

	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset(&pthread_mutex_trylock_4_3_usersigs);
	ret = sigaddset(&pthread_mutex_trylock_4_3_usersigs, SIGUSR1);
	ret |= sigaddset(&pthread_mutex_trylock_4_3_usersigs, SIGUSR2);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to add SIGUSR1 or 2 to a signal set");  }
	
	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &pthread_mutex_trylock_4_3_usersigs, NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Unable to block SIGUSR1 and SIGUSR2 in main thread");  }

	#ifdef WITH_SYNCHRO
	if (sem_init(&pthread_mutex_trylock_4_3_count_semsig1, 0, 1))
	{ UNRESOLVED(errno, "Semsig1  init"); }
	if (sem_init(&pthread_mutex_trylock_4_3_count_semsig2, 0, 1))
	{ UNRESOLVED(errno, "Semsig2  init"); }
	#endif

	if ((ret = pthread_create(&th_work, NULL, pthread_mutex_trylock_4_3_test, NULL)))
	{ UNRESOLVED(ret, "Worker thread creation failed"); }
	
	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &pthread_mutex_trylock_4_3_count_semsig1;
	arg2.sem = &pthread_mutex_trylock_4_3_count_semsig2;
#endif


	if ((ret = pthread_create(&th_sig1, NULL, pthread_mutex_trylock_4_3_sendsig, (void *)&arg1)))
	{ UNRESOLVED(ret, "Signal 1 sender thread creation failed"); }
	if ((ret = pthread_create(&th_sig2, NULL, pthread_mutex_trylock_4_3_sendsig, (void *)&arg2)))
	{ UNRESOLVED(ret, "Signal 2 sender thread creation failed"); }


	/* Let's wait for a while now */
	sleep(1);
	

	/* Now stop the threads and join them */
	do { pthread_mutex_trylock_4_3_do_it=0; }
	while (pthread_mutex_trylock_4_3_do_it);
	
	if ((ret = pthread_join(th_sig1, NULL)))
	{ UNRESOLVED(ret, "Signal 1 sender thread join failed"); }
	if ((ret = pthread_join(th_sig2, NULL)))
	{ UNRESOLVED(ret, "Signal 2 sender thread join failed"); }
	
	if ((ret = pthread_join(th_work, NULL)))
	{ UNRESOLVED(ret, "Worker thread join failed"); }

	#if VERBOSE > 0
	pthread_mutex_trylock_output("Test executed successfully.\n");
	pthread_mutex_trylock_output("  %d mutex locks.\n", pthread_mutex_trylock_4_3_count_ope);
	#ifdef WITH_SYNCHRO
	pthread_mutex_trylock_output("  %d signals were sent meanwhile.\n", pthread_mutex_trylock_4_3_count_sig);
	#endif 
	#endif	
	PASSED;
}


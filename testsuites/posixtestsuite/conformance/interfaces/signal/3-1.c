/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program tests the assertion that the signal shall be ignored
 if the value of the func parameter is SIG_IGN.

 How this program tests this assertion is by setting up a handler 
 "myhandler" for SIGCHLD, and then raising that signal. If the 
 handler_called variable is anything but 1, then fail, otherwise pass.
     
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

static int handler_called = 0;

static void myhandler(int signo)
{
	printf("SIGCHLD called. Inside handler\n");
	handler_called = 1;
}

int signal_3_1()
{
	if (signal(SIGCHLD, myhandler) == SIG_ERR) {
                perror("Unexpected error while using signal()");
               	return PTS_UNRESOLVED;
        }

	raise(SIGCHLD);
	
	if (handler_called != 1) {
		printf("Test FAILED: handler was called even though default was expected\n");
		return PTS_FAIL;
	}		
	return PTS_PASS;
} 

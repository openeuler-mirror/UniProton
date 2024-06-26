/*   
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

   Testing trying to ignore a signal that cannot be ignored.
   Line 1264 in Issue 6 of the Posix System Interfaces document says that 
   the system shall not allow the signals SIGKILL or SIGSTOP
   to be ignored.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include "posixtest.h"

int sigignore_6_1()
{
	if (sigignore(SIGKILL) == -1) {
		if (EINVAL == errno) {
			printf ("errno set to EINVAL\n");
			return PTS_PASS;
		} else {
			printf ("errno not set to EINVAL\n");
			return PTS_FAIL;
		}
	}
	
	printf("sigignore did not return -1\n");
	return PTS_FAIL;
}

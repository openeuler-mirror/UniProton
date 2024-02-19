#include <sys/resource.h>
#include <pthread.h>
#include <unistd.h>

int setpriority(int which, id_t who, int prio)
{
	struct sched_param param;
	int ret, policy;

	(void)which;

	if (who == 0) {
		who = getpid();
	}

	ret = pthread_getschedparam((pthread_t)who, &policy, &param);
	if (ret < 0) {
		return ret;
	}

	param.sched_priority = prio;

	return pthread_setschedparam((pthread_t)who, policy, &param);
}

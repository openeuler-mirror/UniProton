#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "test.h"

static const char path[] = ".";
static const int id = 'h';

#define T(f) do{ \
	if ((f)+1 == 0) \
		t_error("%s failed: %s\n", #f, strerror(errno)); \
}while(0)

#define EQ(a,b,fmt) do{ \
	if ((a) != (b)) \
		t_error("%s == %s failed: " fmt "\n", #a, #b, a, b); \
}while(0)

static void set()
{
	time_t t;
	key_t k = 1234;
	int shmid;
	struct shmid_ds shmid_ds;
	void *p;

	if (shmget(k, 100, 0) != -1 || errno != ENOENT)
		t_error("shmget(k, 100, 0) should have failed with ENOENT, got %s\n", strerror(errno));

	if (shmget(k, 0, IPC_CREAT|0666) != -1 || errno != EINVAL)
		t_error("shmget(k, 0, IPC_CREAT) should have failed with EINVAL, got %s\n", strerror(errno));

	T(t = time(0));

	/* make sure we get a clean shared memory id */
	T(shmid = shmget(k, 100, IPC_CREAT|0666));
	T(shmctl(shmid, IPC_RMID, 0));

	if (shmat(shmid, 0, 0) != (void *)-1 || errno != EINVAL)
		t_error("shmat(shmid, 0, 0) should have failed with EINVAL, got %s\n", strerror(errno));

	T(shmid = shmget(k, 100, IPC_CREAT|IPC_EXCL|0666));

	if (t_status)
		exit(t_status);

	/* check IPC_EXCL */
	errno = 0;
	if (shmget(k, 100, IPC_CREAT|IPC_EXCL|0666) != -1 || errno != EEXIST)
		t_error("shmget(IPC_CREAT|IPC_EXCL) should have failed with EEXIST, got %s\n", strerror(errno));

	/* check if shmget initilaized the msshmid_ds structure correctly */
	T(shmctl(shmid, IPC_STAT, &shmid_ds));
	EQ(shmid_ds.shm_perm.mode & 0x1ff, 0666, "got %o, want %o");
	EQ(shmid_ds.shm_segsz, 100, "got %d, want %d");
	EQ(shmid_ds.shm_lpid, 0, "got %d, want %d");
	EQ(shmid_ds.shm_cpid, getpid(), "got %d, want %d");
	EQ((int)shmid_ds.shm_nattch, 0, "got %d, want %d");
	EQ((long long)shmid_ds.shm_atime, 0, "got %lld, want %d");
	EQ((long long)shmid_ds.shm_dtime, 0, "got %lld, want %d");
	if (shmid_ds.shm_ctime < t)
		t_error("shmid_ds.shm_ctime >= t failed: got %lld, want >= %lld\n", (long long)shmid_ds.shm_ctime, (long long)t);
	if (shmid_ds.shm_ctime > t+5)
		t_error("shmid_ds.shm_ctime <= t+5 failed: got %lld, want <= %lld\n", (long long)shmid_ds.shm_ctime, (long long)t+5);

	/* test attach */
	if ((p=shmat(shmid, 0, 0)) == 0)
		t_error("shmat failed: %s\n", strerror(errno));
	T(shmctl(shmid, IPC_STAT, &shmid_ds));
	EQ((int)shmid_ds.shm_nattch, 1, "got %d, want %d");
	EQ(shmid_ds.shm_lpid, getpid(), "got %d, want %d");
	if (shmid_ds.shm_atime < t)
		t_error("shm_atime is %lld want >= %lld\n", (long long)shmid_ds.shm_atime, (long long)t);
	if (shmid_ds.shm_atime > t+5)
		t_error("shm_atime is %lld want <= %lld\n", (long long)shmid_ds.shm_atime, (long long)t+5);
	strcpy(p, "test data");
	T(shmdt(p));
}

static void *get(void *arg)
{
	key_t k = 1234;
	int shmid;
	void *p;

	T(shmid = shmget(k, 0, 0));

	errno = 0;
	if ((p=shmat(shmid, 0, SHM_RDONLY)) == 0)
		t_error("shmat failed: %s\n", strerror(errno));

	if (strcmp(p, "test data") != 0)
		t_error("reading shared mem failed: got \"%.100s\" want \"test data\"\n", p);

	/* cleanup */
	T(shmdt(p));
	T(shmctl(shmid, IPC_RMID, 0));
}

int ipc_shm_test(void)
{
	int p;
	pthread_t new_th;

	set();
	p = pthread_create(&new_th, NULL, get, NULL);

	if (p != 0)
		t_error("fork failed: %s\n", strerror(errno));
	else {
		p = pthread_join(new_th, NULL);
		T(p);
	}
	return t_status;
}

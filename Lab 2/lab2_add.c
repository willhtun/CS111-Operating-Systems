
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Global
int num_iterations = 1;
int opt_yield = 0;
int opt_mutex = 0;
int opt_spinlock = 0;
int spinlock_key = 0;
int opt_compareAndSwap = 0;
pthread_mutex_t lock;

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
}

void mutex_add(long long *pointer, long long value) {
	pthread_mutex_lock(&lock);
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
	pthread_mutex_unlock(&lock);
}

void spinlock_lock(int * loc) {
	while (__sync_lock_test_and_set(loc, 1)) {}
}

void spinlock_unlock(int * loc) {
	__sync_lock_release(loc);
}

void spinlock_add(long long *pointer, long long value) {
	spinlock_lock(&spinlock_key);
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
	spinlock_unlock(&spinlock_key);
}

void compareswap_add(long long *pointer, long long value) {
	int original;
	int new;
	do
	{
		original = *pointer;
		if (opt_yield)
		{
			sched_yield();
		}
		new = original + value;
	} while (__sync_val_compare_and_swap(pointer, original, new) != original);
}

void* thread_add(void *ctr) {
	int a;
	if (opt_mutex) {
		for (a = 0; a < num_iterations; a++)
			mutex_add((long long*)ctr, 1);
		for (a = 0; a < num_iterations; a++)
			mutex_add((long long*)ctr, -1);
	}
	else if (opt_spinlock) {
		for (a = 0; a < num_iterations; a++)
			spinlock_add((long long*)ctr, 1);
		for (a = 0; a < num_iterations; a++)
			spinlock_add((long long*)ctr, -1);
	}
	else if (opt_compareAndSwap) {
		for (a = 0; a < num_iterations; a++)
			compareswap_add((long long*)ctr, 1);
		for (a = 0; a < num_iterations; a++)
			compareswap_add((long long*)ctr, -1);
	}
	else {
		for (a = 0; a < num_iterations; a++)
			add((long long*)ctr, 1);
		for (a = 0; a < num_iterations; a++)
			add((long long*)ctr, -1);
	}
	return NULL;
}

int main(int argc, char *argv[]) {

	// Variables
	int opt;
	int num_threads = 1;
	int num_operations = 2;
	long long int counter;
	char operationName[100] = "add";
	struct timespec highres_starttime, highres_endtime, highres_runtime;

	// Arguments
	static struct option long_options[] = {
		{ "threads", required_argument, 0, 't' },
		{ "iterations", required_argument, 0, 'i' },
		{ "yield", no_argument, 0, 'y'},
		{ "sync", required_argument, 0, 's' },
		{ 0, 0, 0, 0 }
	};
	while ((opt = getopt_long(argc, argv, "t:i:ys:", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 't':
			num_threads = strtol(optarg, NULL, 10);
			break;
		case 'i':
			num_iterations = strtol(optarg, NULL, 10);
			break;
		case 'y':
			strcat(operationName, "-yield");
			opt_yield = 1;
			break;
		case 's':
			if (optarg[0] == 'm')
				opt_mutex = 1;
			else if (optarg[0] == 's')
				opt_spinlock = 1;
			else if (optarg[0] == 'c')
				opt_compareAndSwap = 1;
			else {
				printf("%s", "Use sync option of -m, -s, or -c.\n");
				exit(1);
			}
			break;
		default:
			printf("%s", "Undefined argument. Use --threads=#, or --iterations=#. If no arguments are used, the default is 1 for both number of threads and number of iterations.\n");
			exit(1);
		}
	}

	counter = 0;
	clock_gettime(CLOCK_REALTIME, &highres_starttime);
	pthread_t thread_id[num_threads];
	int i;
	for (i = 0; i < num_threads; i++) {
		pthread_create(&thread_id[i], NULL, thread_add, &counter);
	}
	for (i = 0; i < num_threads; i++) {
		pthread_join(thread_id[i], NULL);
	}
	clock_gettime(CLOCK_REALTIME, &highres_endtime);
	highres_runtime.tv_nsec = highres_endtime.tv_nsec - highres_starttime.tv_nsec;

	if (opt_mutex)
		strcat(operationName, "-m");
	else if (opt_spinlock)
		strcat(operationName, "-s");
	else if (opt_compareAndSwap)
		strcat(operationName, "-c");
	else
		strcat(operationName, "-none");

	num_operations = num_threads * num_iterations * 2;
	long long int avgRuntime = highres_runtime.tv_nsec / num_operations;
	printf("%s%s%d%s%d%s%d%s%d%s%d%s%lld\n", operationName, ",", num_threads, ",", num_iterations, ",", num_operations, ",", (int) (highres_runtime.tv_nsec), ",", (int) (avgRuntime), ",", counter);
	exit(0);
}
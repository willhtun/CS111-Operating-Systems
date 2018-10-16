#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "SortedList.h"

SortedList_t* list;
SortedListElement_t* elementList;
int num_lists = 1;
int num_threads = 1;
int num_iterations = 1;
long long avgLockTime = 0;
int usingLock;
int opt_sync_m = 0, opt_sync_s = 0;

// 2B
typedef struct list_cut {
	SortedList_t* list;
	int s_lock;
	pthread_mutex_t m_lock;
} list_cut;
typedef struct list_all {
	int num_lists;
	list_cut* lists;
} list_all;
list_all listAll;

// Parameters
struct parameters {
	SortedListElement_t* p_elements;
	int p_lockID;
};

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
int spinlock = 0;

unsigned long hasher_func(const char * key) { // Change?
	uint32_t m_hash, i;
	unsigned int len = strlen(key);
	for (m_hash = i = 0; i < len; ++i)
	{
		m_hash += key[i];
		m_hash += (m_hash << 10);
		m_hash ^= (m_hash >> 6);
	}
	m_hash += (m_hash << 3);
	m_hash ^= (m_hash >> 11);
	m_hash += (m_hash << 15);
	return m_hash;
}

void spinlock_time(int idx, struct timespec * total) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	while (__sync_lock_test_and_set(&listAll.lists[idx].s_lock, 1) == 1); //spin
	clock_gettime(CLOCK_MONOTONIC, &end);
	total->tv_sec += end.tv_sec - start.tv_sec;
	total->tv_nsec += end.tv_nsec - start.tv_nsec;
}

void spinunlock(int idx) {
	__sync_lock_release(&listAll.lists[idx].s_lock);
}

void mutexlock_time(int idx, struct timespec *total) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	pthread_mutex_lock(&listAll.lists[idx].m_lock);
	clock_gettime(CLOCK_MONOTONIC, &end);
	total->tv_sec += end.tv_sec - start.tv_sec;
	total->tv_nsec += end.tv_nsec - start.tv_nsec;
}

void mutexunlock(int idx) {
	pthread_mutex_unlock(&listAll.lists[idx].m_lock);
}

void* listOpt_noOptions(void* arg, struct timespec *total) {
	SortedListElement_t *element = arg;
	int i;
	int length = 0;

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		SortedList_insert(listAll.lists[index].list, element + i);
	}

	for (i = 0; i < listAll.num_lists; i++) {
		int tmp = SortedList_length(listAll.lists[i].list);
		if (tmp < 0) {
			printf("%s", "Length should never be less than 0.\n");
			exit(2);
		}
		length += tmp;
	}
	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		SortedListElement_t* find = SortedList_lookup(listAll.lists[index].list, element[i].key);
		if (find == NULL) {
			printf("%s", "Element not found.\n");
			exit(2);
		}
		int x = SortedList_delete(find);
		if (x != 0) {
			printf("%s", "Deleting element failed.\n");
			exit(2);
		}
	}
	return total;
}

void* listOpt_mutex(void* arg, struct timespec *total) {
	SortedListElement_t *element = arg;
	int i;
	int length = 0;

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		mutexlock_time(index, total);
		SortedList_insert(listAll.lists[index].list, element + i);
		mutexunlock(index);
	}

	for (i = 0; i < listAll.num_lists; i++) {
		mutexlock_time(i, total);
		int tmp = SortedList_length(listAll.lists[i].list);
		if (tmp < 0) {
			printf("%s", "Length should never be less than 0.\n");
			exit(2);
		}
		length += tmp;
		mutexunlock(i);
	}
	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		mutexlock_time(index, total);
		SortedListElement_t* find = SortedList_lookup(listAll.lists[index].list, element[i].key);
		if (find == NULL) {
			printf("%s", "Element not found.\n");
			exit(2);
		}
		int x = SortedList_delete(find);
		if (x != 0) {
			printf("%s", "Deleting element failed.\n");
			exit(2);
		}
		mutexunlock(index);
	}
	return total;
}

void* listOpt_spin(void* arg, struct timespec *total) {
	SortedListElement_t *element = arg;
	int i;
	int length = 0;

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		spinlock_time(index, total);
		SortedList_insert(listAll.lists[index].list, element + i);
		spinunlock(index);
	}

	for (i = 0; i < listAll.num_lists; i++) {
		spinlock_time(i, total);
		int tmp = SortedList_length(listAll.lists[i].list);
		if (tmp < 0) {
			printf("%s", "Length should never be less than 0.\n");
			exit(2);
		}
		length += tmp;
		spinunlock(i);
	}
	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}

	for (i = 0; i < num_iterations; i++) {
		unsigned long index = hasher_func((element + i)->key) % listAll.num_lists;
		spinlock_time(index, total);
		SortedListElement_t* find = SortedList_lookup(listAll.lists[index].list, element[i].key);
		if (find == NULL) {
			printf("%s", "Element not found.\n");
			exit(2);
		}
		int x = SortedList_delete(find);
		if (x != 0) {
			printf("%s", "Deleting element failed.\n");
			exit(2);
		}
		spinunlock(index);
	}
	return total;
}

void * listOpt(void *arg) {
	struct timespec *totaltime = NULL;
	if (usingLock)
		totaltime = calloc(1, sizeof *totaltime);
	if (opt_sync_m)
		return listOpt_mutex(arg, totaltime);
	else if (opt_sync_s)
		return listOpt_spin(arg, totaltime);
	else
		return listOpt_noOptions(arg, totaltime);
}

int main(int argc, char *argv[]) {
	// Variables
	int opt;
	int opt_yield = 0, opt_yield_i = 0, opt_yield_d = 0, opt_yield_l = 0;
	int num_operations;
	char operationName[100] = "list";
	char yieldName[10] = "-none";
	char syncName[10] = "-none";

	int numNodes = 1;
	usingLock = 0;

	struct timespec highres_starttime, highres_endtime, highres_runtime;
	struct timespec lock_wait;

	// Arguments
	static struct option long_options[] = {   
		{ "threads", required_argument, 0, 't' },
		{ "iterations", required_argument, 0, 'i' },
		{ "yield", required_argument, 0, 'y' },
		{ "sync", required_argument, 0, 's' },
		{ "lists", required_argument, 0, 'l' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "t:i:y:s:l:", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 't':
			num_threads = strtol(optarg, NULL, 10);
			break;
		case 'i':
			num_iterations = strtol(optarg, NULL, 10);
			break;
		case 'y':
			strcpy(yieldName, "-");
			opt_yield = 1;
			int i;
			for (i = 0; i < (int)strlen(optarg); i++) {
				if (optarg[i] == 'i') {
					opt_yield_i = 1;
					opt_yield |= INSERT_YIELD;
				}
				else if (optarg[i] == 'd') {
					opt_yield_d = 1;
					opt_yield |= DELETE_YIELD;
				}
				else if (optarg[i] == 'l') {
					opt_yield_l = 1;
					opt_yield |= LOOKUP_YIELD;
				}
				else {
					printf("%s", "Undefined option for yield. Use --yield=[idl].\n");
					exit(1);
				}
			}
			break;
		case 's':
			usingLock = 1;
			if (strlen(optarg) > 1) {
				printf("%s", "Undefined option for sync. Use --sync=[sm].\n");
				exit(1);
			}
			strcpy(syncName, "-");
			if (optarg[0] == 's')
				opt_sync_s = 1;
			else if (optarg[0] == 'm')
				opt_sync_m = 1;
			else {
				printf("%s", "Undefined option for sync. Use --sync=[sm].\n");
				exit(1);
			}
			break;
		case 'l':
			num_lists = strtol(optarg, NULL, 10);
			break;
		default:
			printf("%s", "Undefined argument. Use --threads=#, --iterations=#, or --yield=[idl]. If no arguments are used, the default is 1 for both number of threads and number of iterations.\n");
			exit(1);
		}
	}

	// Program starts
	int i = 0;

	// Initializes list
	listAll.num_lists = num_lists;
	listAll.lists = calloc(num_lists, sizeof *listAll.lists);

	for (i = 0; i < num_lists; i++) {
		list_cut * temp_list = (listAll.lists + i);
		temp_list->list = malloc(sizeof(SortedList_t));
		temp_list->list->key = NULL;
		temp_list->list->next = temp_list->list->prev = temp_list->list;
		pthread_mutex_init(&temp_list->m_lock, NULL);
		temp_list->s_lock = 0;
	}


	// Initializes elements
	srand(time(NULL));
	numNodes = num_threads * num_iterations;
	elementList = malloc(numNodes * sizeof(SortedListElement_t));
	for (i = 0; i < numNodes; i++) {
		char *randKey = malloc(4 * sizeof(char));
		int randNum = rand() % 999 + 100;
		snprintf(randKey, 4, "%d", randNum);
		elementList[i].key = randKey;
		elementList[i].next = &elementList[i];
		elementList[i].prev = &elementList[i];
	}
	
	// Record start time
	clock_gettime(CLOCK_MONOTONIC, &highres_starttime);
	pthread_t *thread_id = malloc(num_threads * sizeof(pthread_t));

	// Create threads
	for (i = 0; i < num_threads; i++) {
		pthread_create(&thread_id[i], NULL, listOpt, elementList + (i*num_iterations));
	}

	// Join threads
	for (i = 0; i < num_threads; i++) {
		void *total_time;
		pthread_join(thread_id[i], &total_time);
		if (total_time) {
			lock_wait.tv_sec += ((struct timespec *)total_time)->tv_sec;
			lock_wait.tv_nsec += ((struct timespec *)total_time)->tv_nsec;
		}
	}

	// Record end time
	clock_gettime(CLOCK_MONOTONIC, &highres_endtime);
	highres_runtime.tv_sec = highres_endtime.tv_sec - highres_starttime.tv_sec;
	highres_runtime.tv_nsec = highres_endtime.tv_nsec - highres_starttime.tv_nsec;
	long long totalWaitTime = highres_runtime.tv_nsec + (1000000000L * (long long)highres_runtime.tv_sec);

	// Record total locking time
	long long totalLockTime = lock_wait.tv_nsec + (1000000000L * (long long)lock_wait.tv_sec);
	avgLockTime = totalLockTime / ((3 * num_iterations + 1) * num_threads);


	// Print CSV
	num_operations = num_threads * num_iterations * 3;

	if (opt_yield) {
		if (opt_yield_i) 
			strcat(yieldName, "i");
		if (opt_yield_d)
			strcat(yieldName, "d");
		if (opt_yield_l)
			strcat(yieldName, "l");
	}
	if (opt_sync_m)
		strcat(syncName, "m");
	if (opt_sync_s)
		strcat(syncName, "s");
		
	strcat(operationName, yieldName);
	strcat(operationName, syncName);
	printf("%s%s%d%s%d%s%d%s%d%s%lld%s%lld%s%lld\n", operationName, ",", num_threads, ",", num_iterations, ",", num_lists, ",", num_operations, ",", totalWaitTime, ",", (totalWaitTime / num_operations), ",", avgLockTime);
	exit(0);
}
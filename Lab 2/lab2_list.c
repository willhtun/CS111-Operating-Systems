#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "SortedList.h"

SortedList_t* list;
SortedListElement_t* elementList;
int num_lists = 1;
int num_threads = 1;
int num_iterations = 1;

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
int spinlock = 0;

void* listOpt_noOptions(void* arg) {
	SortedListElement_t* elements = arg;
	int length;
	SortedListElement_t* toDel;

	int i;
	for (i = 0; i < num_iterations; i++) {
		SortedList_insert(list, elements + i);
	}
	length = SortedList_length(list);
	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}
	for (i = 0; i < num_iterations; i++) {
		toDel = SortedList_lookup(list, elements[i].key);
		if (toDel != NULL) {
			int delStatus = SortedList_delete(&elements[i]);
			if (delStatus != 0) {
				printf("%s", "Error deleting element from list.\n");
				exit(2);
			}
		} 
		else {
			printf("%s", "No such element found in list.\n");
			exit(2);
		}
	}
	return NULL;
}

void* listOpt_mutex(void* arg) {
	SortedListElement_t* elements = arg;
	int length;
	SortedListElement_t* toDel;

	int i;
	for (i = 0; i < num_iterations; i++) {
		pthread_mutex_lock(&mlock);
		SortedList_insert(list, elements + i);
		pthread_mutex_unlock(&mlock);
	}

	pthread_mutex_lock(&mlock);
	length = SortedList_length(list);
	pthread_mutex_unlock(&mlock);

	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}
	for (i = 0; i < num_iterations; i++) {
		pthread_mutex_lock(&mlock);
		toDel = SortedList_lookup(list, elements[i].key);
		if (toDel != NULL) {
			int delStatus = SortedList_delete(&elements[i]);
			if (delStatus != 0) {
				printf("%s", "Error deleting element from list.\n");
				exit(2);
			}
		}
		else {
			printf("%s", "No such element found in list.\n");
			exit(2);
		}
		pthread_mutex_unlock(&mlock);
	}
	return NULL;
}

void* listOpt_spin(void* arg) {
	SortedListElement_t* elements = arg;
	int length;
	SortedListElement_t* toDel;

	int i;
	for (i = 0; i < num_iterations; i++) {
		while (__sync_lock_test_and_set(&spinlock, 1) == 1);	
		SortedList_insert(list, elements + i);
		__sync_lock_release(&spinlock);
	}

	while (__sync_lock_test_and_set(&spinlock, 1) == 1);
	length = SortedList_length(list);
	__sync_lock_release(&spinlock);

	if (length < 0) {
		printf("%s", "Length should never be less than 0.\n");
		exit(2);
	}
	for (i = 0; i < num_iterations; i++) {
		while (__sync_lock_test_and_set(&spinlock, 1) == 1);
		toDel = SortedList_lookup(list, elements[i].key);

		if (toDel != NULL) {
			int delStatus = SortedList_delete(&elements[i]);
			if (delStatus != 0) {
				printf("%s", "Error deleting element from list.\n");
				exit(2);
			}
		}
		else {
			printf("%s", "No such element found in list.\n");
			exit(2);
		}
		__sync_lock_release(&spinlock);
	}
	return NULL;
}


int main(int argc, char *argv[]) {
	// Variables
	int opt;
	int opt_yield = 0, opt_yield_i = 0, opt_yield_d = 0, opt_yield_l = 0, opt_sync_m = 0, opt_sync_s = 0;
	int num_operations;
	char operationName[100] = "list";
	char yieldName[10] = "-none";
	char syncName[10] = "-none";

	int numNodes = 1;
	int listLength = 0;

	struct timespec highres_starttime, highres_endtime, highres_runtime;

	// Arguments
	static struct option long_options[] = {
		{ "threads", required_argument, 0, 't' },
		{ "iterations", required_argument, 0, 'i' },
		{ "yield", required_argument, 0, 'y' },
		{ "sync", required_argument, 0, 's' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "t:i:y:s:", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
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
			for (i = 0; i < (int) strlen(optarg); i++) {
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
		default:
			printf("%s", "Undefined argument. Use --threads=#, --iterations=#, or --yield=[idl]. If no arguments are used, the default is 1 for both number of threads and number of iterations.\n");
			exit(1);
		}
	}

	// Program starts

	// Initializes list
	list = malloc(sizeof(SortedList_t));
	list->next = list;
	list->prev = list;
	list->key = NULL;

	// Initializes elements
	srand(time(NULL));
	numNodes = num_threads * num_iterations;
	elementList = malloc(numNodes * sizeof(SortedListElement_t));
	int i = 0;
	for (i = 0; i < numNodes; i++) {

		char *randKey = malloc(4 * sizeof(char));
		int randNum = rand() % 999 + 100;
		snprintf(randKey, 4, "%d", randNum);
		elementList[i].key = randKey;
		elementList[i].next = &elementList[i];
		elementList[i].prev = &elementList[i];
	}
	

	// Record start time
	clock_gettime(CLOCK_REALTIME, &highres_starttime);
	pthread_t *thread_id = malloc(num_threads * sizeof(pthread_t));

	// Create threads
	for (i = 0; i < num_threads; i++) {
		if (opt_sync_m)
			pthread_create(&thread_id[i], NULL, listOpt_mutex, elementList + i * num_iterations);
		else if (opt_sync_s)
			pthread_create(&thread_id[i], NULL, listOpt_spin, elementList + i * num_iterations);
		else
			pthread_create(&thread_id[i], NULL, listOpt_noOptions, elementList + i * num_iterations);
	}
	// Join threads
	for (i = 0; i < num_threads; i++) {
		pthread_join(thread_id[i], NULL);
	}

	// Record end time
	clock_gettime(CLOCK_REALTIME, &highres_endtime);
	highres_runtime.tv_nsec = highres_endtime.tv_nsec - highres_starttime.tv_nsec;


	listLength = SortedList_length(list);
	if (listLength != 0) {
		printf("%s", "Lenght of list is not zero due to synchronization error(s).\n");
		exit(2);
	}

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
	printf("%s%s%d%s%d%s%d%s%d%s%d%s%d\n", operationName, ",", num_threads, ",", num_iterations, ",", num_lists, ",", num_operations, ",", (int) highres_runtime.tv_nsec, ",", (int)((highres_runtime.tv_nsec) / num_operations));
	exit(0);
}
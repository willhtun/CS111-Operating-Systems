#include "SortedList.h"
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	SortedListElement_t* p = list->next;
	while (p->key != NULL && strcmp(p->key, element->key) < 0)
		p = p->next;

	p->prev->next = element;
	element->prev = p->prev;
	if (INSERT_YIELD)
		sched_yield();
	element->next = p;
	p->prev = element;
	return;
}

int SortedList_delete(SortedListElement_t *element) {
	if (element == NULL)
		return 1;
	if (element->next->prev != element)
		return 1;
	if (element->prev->next != element)
		return 1;
	if (DELETE_YIELD)
		sched_yield();
	element->prev->next = element->next;
	element->next->prev = element->prev;
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	SortedListElement_t* p = list;
	if (p == NULL || key == NULL)
		return NULL;
	if (p->key == key)
		return list;
	while (p->next != p) {
		if (strcmp(p->next->key, key) > 0) {
			return NULL; //because ascending
		}
		else if (strcmp(p->next->key, key) == 0) {
			return p->next;
		}
		if (LOOKUP_YIELD)
			sched_yield();
		p = p->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	int count = 0;
	SortedListElement_t* p = list;
	if (p == NULL)
		return -1;
	while (p->next != list) {
		if (p->next->prev->next != p->next || p->next->next->prev != p->next)
			return -1;
		count += 1;
		if (LOOKUP_YIELD)
			sched_yield();
		p = p->next;
	}
	return count;
}
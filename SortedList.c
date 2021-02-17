//NAME: Khoa Quach
//EMAIL: khoaquachschool@gmail.com
//ID: 105123806

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include "SortedList.h"
#include <string.h>
//int opt_yield = 0;
/* • insert an element into a sorted list*/
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	if(list == NULL || element == NULL)
		return;
	if(opt_yield & INSERT_YIELD){
		sched_yield();
	}
	/// empty

	if(list->next == NULL || (list->next == list && list->prev == list)){
		list->next = element;
		list->prev = element;
		element->next = list;
		element->prev = list;
		return;
	}
	SortedListElement_t *curr = list->next;
	while (curr != list && strcmp(element->key, curr->key) >= 0){
		curr = curr->next;
	}
	
	element->next = curr;
	element->prev = curr->prev;
	curr->prev->next = element;
	curr->prev = element;
}


/*
TA slides:
void list_remove(node *element) {
	node*n = element->next; 
	node*p = element->prev;
	n->prev = p;
	p->next = n;
	element->next = NULL;
	element->prev = NULL;
}

*/
int SortedList_delete(SortedListElement_t *element)
{
	// error check
	if(element == NULL){
		return 1;
	}
	// error check
	if((element->next->prev != element) || (element->prev->next != element)){
		return 1;
	}
	if(opt_yield & DELETE_YIELD){
		sched_yield();
	}
	element->next->prev = element->prev;
	element->prev->next = element->next;
	return 0;
}

/*
search sorted list for a key
specified list will be searched for an element with the specified key
*/
SortedListElement_t  *SortedList_lookup(SortedList_t *list, const char *key)
{
	if (list == NULL)
		return NULL;

	SortedListElement_t *curr = list->next;
	while(curr!=NULL && curr!=list){
		if (opt_yield & LOOKUP_YIELD){
			sched_yield();
		}
		if(strcmp(curr->key,key) == 0)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}



/*
count elements in a sorted list while enumerating list
checks all prev/next pointers
*/
int SortedList_length(SortedList_t *list)
{
	if(list == NULL)
		return -1;
	//
	int length = 0;
	SortedListElement_t* curr = list->next;
	//
	while (curr!=NULL && curr!=list){
		if (opt_yield & LOOKUP_YIELD){
			sched_yield();
		}
		length++;
		curr = curr->next;
	}
	return length;
}
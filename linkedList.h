/*
 * LINKED LIST HEADER FILE
 * Integer Values Stored
 *
 * Exit Error 20 indicates error with malloc
 * */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#ifndef TYPE
#define TYPE pid_t
#endif

// Header Files
#include <stdlib.h>
#include <stdio.h>

/* Main Struct for Linked List */
struct LinkedList
{
	struct Node * head;
	struct Node * tail;
	int count;
};

/* Each Node of Linked List*/
struct Node
{
	struct Node * next;
	TYPE val;
};

// List function prototypes
void initList(struct LinkedList * list);
void pushList(struct LinkedList * list, TYPE val);
void removeFromList(struct LinkedList * list, TYPE val);
int listContains(struct LinkedList * list, TYPE val);					
int getSize(struct LinkedList * list);
void printList(struct LinkedList * list);
void freeList(struct LinkedList * list);

/* Iterator Struct */
struct ListIter
{
	struct Node * curNode;
};

// Iterator function prototypes
// listIterNext() increments the iterator
// next() and hasNext() must be interleaved
void initListIter(struct ListIter * iter, struct LinkedList * list);
int listIterHasNext(struct ListIter * iter);
TYPE listIterNext(struct ListIter * iter);

#endif

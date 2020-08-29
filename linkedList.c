/*
 * LINKED LIST/LINKED LIST ITERATOR IMPLEMENTATION
 * Integer Values Stored
 *
 * */

// Header Files
#include "linkedList.h"
#include <stdlib.h>
#include <stdio.h>

/******************************** LINKED LIST FUNCTIONS *********************************************/

/*
 * INITIALIZE LINKED LIST
 * Uses one sentinel at the head
 * */
void initList(struct LinkedList * list)
{
	// Allocate head sentinel
	list->head = malloc(sizeof(struct Node));
	if(list->head == NULL) exit(20);

	// Initialize variables
	list->head->next = NULL;
	list->tail = list->head;
	list->count = 0;
}

/*
 * PUSH ONTO END OF LIST
 * */
void pushList(struct LinkedList * list, TYPE val)
{
	// Allocate new node
	struct Node * newNode = malloc(sizeof(struct Node));
	if(newNode == NULL) exit(20);
	newNode->val = val;
	newNode->next = NULL;

	// Add node to list
	list->tail->next = newNode;
	list->tail = newNode;

	// Update count
	list->count++;
}

/*
 * REMOVE VALUE FROM LIST, IF EXISTS
 * */
void removeFromList(struct LinkedList * list, TYPE val)
{
	// Initialize temp variables
	struct Node * curNode = list->head;
	struct Node * temp = NULL;

	// Loop through next nodes
	while(curNode->next != NULL)
	{
		// If the next node has the value
		if(curNode->next->val == val)
		{
			// Free the next node and update the list
			temp = curNode->next;
			curNode->next = curNode->next->next;

			// Check if it's the tail
			if(list->tail == temp)
				list->tail = curNode;

			free(temp);
			temp = NULL;

			// Update list count
			list->count--;

			return;
		}
		else
		{
			curNode = curNode->next;
		}
	}
}

/*
 * CHECK IF VALUE IS IN LIST
 * */
int listContains(struct LinkedList * list, TYPE val)
{
	// Initialize temp variables
	struct Node * curNode = list->head->next;

	// Loop through next nodes
	while(curNode != NULL)
	{
		// If this node has the value
		if(curNode->val == val)
		{
			return 1;
		}
		curNode = curNode->next;
	}

	// Else, value not found
	return 0;
}

/*
 * GET SIZE OF LIST
 * */
int getSize(struct LinkedList * list)
{
	return list->count;
}

/*
 * PRINT LIST CONTENTS
 * */
void printList(struct LinkedList * list)
{
	// Initialize temp variable
	struct Node * curNode = list->head->next;

	// Loop through next nodes
	while(curNode != NULL)
	{
		printf("%d ", curNode->val);
		fflush(stdout);
		curNode = curNode->next;
	}

	// Newline at end
	printf("\n");
	fflush(stdout);
}

/*
 * FREE LINKED LIST MEMORY
 * */
void freeList(struct LinkedList * list)
{
	// Initialize temp variables
	struct Node * curNode = list->head;
	struct Node * temp = NULL;

	// Loop through nodes including head
	while(curNode != NULL)
	{
		temp = curNode;
		curNode = curNode->next;

		// Free them all!
		free(temp);
		temp = NULL;
	}
}


/******************************** LIST ITERATOR FUNCTIONS *********************************************/

/*
 * INITIALIZE ITERATOR
 * Iterator will hold pointer to Linked List head
 * */
void initListIter(struct ListIter * iter, struct LinkedList * list)
{
	iter->curNode = list->head;
}

/*
 * CHECK IF THERE IS A NEXT ELEMENT
 * */
int listIterHasNext(struct ListIter * iter)
{
	// Check if next node is null
	if(iter->curNode->next != NULL)
		return 1;
	else
		return 0;
}

/*
 * GET NEXT ELEMENT, & MOVE TO NEXT ELEMENT
 * */
TYPE listIterNext(struct ListIter * iter)
{
	// Go to next item
	iter->curNode = iter->curNode->next;
	// return said item
	return iter->curNode->val;
}



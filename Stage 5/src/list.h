/*
// Simply Linked List
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

#ifndef __LIST_H__
#define __LIST_H__

/* An item in the list (a long integer) */
typedef long item_t;

/* A node in the list, holds and item_t and a pointer to the next node */
typedef struct node {
	item_t item;
	struct node *next;
} node_t;

/* The list_t structure, holds the first node in the list */
typedef struct list {
	node_t *first;
} list_t;

/* Creates and returns a new pointer to a list_t structure */
list_t *newList();

/* Adds an item to the given list */
int addItem(list_t*, item_t);

/* Removes an item from the given list */
int removeItem(list_t*, item_t);

/* Destroys and frees any associated resources with the list */
void destroyList(list_t*);

#endif /* __LIST_H__ */
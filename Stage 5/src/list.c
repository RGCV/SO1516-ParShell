/*
// Simply Linked List
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

/*
 ------------------------------------------
 ---------------- HEADERS -----------------
 ------------------------------------------
*/

/* - C/POSIX library headers */
#include <stdlib.h>

/* - Local header files */
#include "list.h"

/*
 ------------------------------------------
 ------------- IMPLEMENTATION -------------
 ------------------------------------------
*/

/* Creates and returns a new pointer to a list_t structure */
list_t *newList() {
	list_t *list = (list_t*)malloc(sizeof(list_t));
	if(list)
		list->first = NULL;
	return list;
}

/* Adds an item to the given list */
int addItem(list_t *list, item_t item) {
	node_t *node = (node_t*)malloc(sizeof(struct node));
	
	if(node) {
		node->item = item;
		node->next = list->first;
		list->first = node;
		return EXIT_SUCCESS;
	}
	
	return EXIT_FAILURE;
}

/* Removes an item from the given list */
int removeItem(list_t *list, item_t item) {
  node_t *node = list->first, *prev = NULL;
  
  while(node) {
    if(node->item == item) {
    	if(node == list->first) {
    		list->first = node->next;
    	} else {
	      prev->next = node->next;
	    }
	    free(node);
      return EXIT_SUCCESS;
    }
    prev = node;
    node = node->next;
  }
  
  return EXIT_FAILURE;
}

/* Destroys and frees any associated resources with the list */
void destroyList(list_t *list) {
	node_t *node = list->first, *temp;
	
	while(node) {
	  temp = node;
	  node = node->next;
	  free(temp);
	}
	
	free(list);
}
/*
 * pidlist.h - definitions and declarations of the integer list 
 */

#ifndef __PIDLIST_H__
#define __PIDLIST_H__

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


/* pidlst_iitem - each element of the list points to the next element */
typedef struct pidlst_iitem {
   pid_t pid;
   time_t starttime;
   time_t endtime;
   int status;
   struct pidlst_iitem *next;
} pidlst_iitem_t;

/* pidlist_t */
typedef struct {
   pidlst_iitem_t * first;
} pidlist_t;


/* lst_new - allocates memory for pidlist_t and initializes it */
pidlist_t* lst_new();

/* pidlst_destroy - free memory of pidlist_t and all its items */
void lst_destroy(pidlist_t * list);

/* insert_new_process - insert a new item with process id and its start time in list 'list' */
int insert_new_process(pidlist_t *list, int pid, time_t starttime);

/* update_teminated_process - updates endtime of element with pid 'pid' */
int update_terminated_process(pidlist_t *list, int pid, int status, time_t endtime);

/* lst_print - print the content of list 'list' to standard output */
void lst_print(pidlist_t *list);

#endif /* __PIDLIST_H__ */
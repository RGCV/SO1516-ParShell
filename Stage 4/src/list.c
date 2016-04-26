/*
 * list.c - implementation of the integer list functions 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>

#include "list.h"

list_t* lst_new()
{
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   return list;
}

void lst_destroy(list_t *list)
{
  lst_iitem_t *item, *nextitem;

  item = list->first;
  while (item != NULL){
    nextitem = item->next;
    free(item);
    item = nextitem;
  }
  free(list);
}

int insert_new_process(list_t *list, int pid, time_t starttime)
{
  lst_iitem_t *item;

  item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
  if(item == NULL)
    return -1;
  else {
    item->pid = pid;
    item->starttime = starttime;
    item->endtime = 0;
    item->status = 0;
    item->next = list->first;
    list->first = item;
  }
  
  return 0;
}

int update_terminated_process(list_t *list, int pid, int status, time_t endtime)
{
  lst_iitem_t *lp = list->first;
  while(lp) {
    if(lp->pid == pid) {
      lp->status = status;
      lp->endtime = endtime;
      return difftime(endtime, lp->starttime);
    }
    
    lp = lp->next;
  }
  
  return -1;
}

void lst_print(list_t *list)
{
  lst_iitem_t *item;

  printf("-- Process list with elapsed time:\n");
  item = list->first;
  while (item){
    if(WIFEXITED(item->status)) {
      fprintf(stderr,  "Process %d exited expectedly (status = %d)",
      item->pid, WEXITSTATUS(item->status));
    }
    else if(WIFSIGNALED(item->status)) {
      fprintf(stderr, "Process %d received a signal (signal = %d)",
      item->pid, WTERMSIG(item->status));
    }
    else {
      fprintf(stderr, "Process %d exited abnormally (status = 0x%08x)",
      item->pid, item->status);
    }
    
    printf(" - Time Elapsed: %02g seconds\n",
      difftime(item->endtime, item->starttime));
    
    item = item->next;
  }
  printf("-- end of list.\n");
}

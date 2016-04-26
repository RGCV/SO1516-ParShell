/**
* Par-shell - Sistemas Operativos 
* Sistemas Operativos, LEIC-A Grupo 38
* 
* @author Rui Ventura,      no. 81045
* @author Goncalo Baptista, no. 81196
* @author Sara Azinhal,     no. 81700
*/

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "commandlinereader.h"
#include "list.h"

#define handle_error(msg) { perror(msg); exit(EXIT_FAILURE); }

#define TRUE 1
#define FALSE 0

#define MAX_ARGS 6
#define EXIT_COMMAND "exit"
#define COMMAND_SIZE 128

int numChildren = 0, monitoring = TRUE;
time_t timestamp;
pthread_mutex_t childMutex = PTHREAD_MUTEX_INITIALIZER;

void *processHandler(void *processes);


int main(int argc, char **argv) {
  pid_t childPid;
  pthread_t pHandler;
  list_t *processes = lst_new();
  char *args[MAX_ARGS + 1], buffer[COMMAND_SIZE];
  
  if(pthread_create(&pHandler, NULL, processHandler, (void *) processes))
    handle_error("pthread_create")
  
  while(TRUE) {
    int numargs;
    numargs = readLineArguments(args, MAX_ARGS + 1, buffer, COMMAND_SIZE);
    
    if(args[0] == NULL) continue;
    if(numargs < 0 || (numargs > 0 && !strcmp(args[0], EXIT_COMMAND))) {
      
      monitoring = FALSE;
      if(pthread_join(pHandler, NULL))
        handle_error("pthread_join");
      
      lst_print(processes);
      lst_destroy(processes);
      return EXIT_SUCCESS;
    }
    else {
      FILE *fp = fopen(args[0], "r"); /* To check file existance */
      if (!fp) perror(args[0]);
      else {
        fclose(fp);
        
        if((childPid = fork()) < 0)
          perror("fork");
        else if(childPid == 0) {
          execv(args[0],args);
          
          handle_error("execv");
        }
        else {
          pthread_mutex_lock(&childMutex);
          timestamp = time(NULL);
          ++numChildren;
          pthread_mutex_unlock(&childMutex);
          
          insert_new_process(processes, childPid, timestamp);
          
        }
      }
    }
  }
}

void *processHandler(void *processes) {
  pid_t childPid;
  int status;
  
  while(TRUE) {
    pthread_mutex_lock(&childMutex);
    if(numChildren > 0) {
      pthread_mutex_unlock(&childMutex);
      childPid = wait(&status);

      pthread_mutex_lock(&childMutex);
      timestamp = time(NULL);
      --numChildren;
      pthread_mutex_unlock(&childMutex);
      
      if(childPid < 0) {
          if(errno == EINTR) /* Process waiting interrupted */
            continue;
          else
            handle_error("wait");
      }
      
      update_terminated_process(processes, childPid, status, timestamp);
    } else if(monitoring) {
      pthread_mutex_unlock(&childMutex);
      sleep(1);
    }
    else break;
  }
  
  return NULL;
}
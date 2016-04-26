#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commandlinereader.h"

#define TRUE 1
#define FALSE 0
#define MAX_ARGS 6

typedef struct process {
    pid_t pid;
    int status;
} process_t;

int main(int argc, char **argv) {
    FILE *fp;
    pid_t pid;
    process_t *psVector;
    int i, status, child_processes = 0;
    char *argVector[MAX_ARGS + 1];
    
    while(TRUE) {
        readLineArguments(argVector, MAX_ARGS + 1);
        
        if(argVector[0] == NULL) continue;
        if(strcmp(argVector[0], "exit") == 0) break;
        else if((fp = fopen(argVector[0], "r")) == NULL) perror(argVector[0]);
        else {
            fclose(fp);
            
            if((pid = fork()) < 0)
                perror("fork");
            else if(pid == 0) {
                execv(argVector[0],argVector);
                
                perror("execv");
                exit(EXIT_FAILURE);
            }
            else child_processes++;
        }
    }
    
    psVector = (process_t*)malloc(sizeof(process_t)*child_processes);
    
    for(i = 0; i < child_processes; i++)
        psVector[i].pid = wait(&(psVector[i].status));
        
    for(i = 0; i < child_processes; i++) {
        pid = psVector[i].pid;
        status = psVector[i].status;
        
        if(WIFEXITED(status))
            fprintf(stderr, "Process %d exited expectedly (status = %d)\n",
                pid, WEXITSTATUS(status));
        else if(WIFSIGNALED(status))
            fprintf(stderr, "Process %d received a signal (%s, signal = %d)\n",
                pid, strsignal(status), WTERMSIG(status));
        else
            fprintf(stderr, "Process %d exited abnormally (status = 0x%x)\n",
                pid, status);
    }
    
    free(psVector);
    
    return EXIT_SUCCESS;
}
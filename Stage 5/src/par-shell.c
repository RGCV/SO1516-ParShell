/*
 +----------------------------------------+
 | Parallel Shell                         |
 | Sistemas Operativos - LEIC-A, Grupo 38 |
 | IST/ULisboa 2015-16                    |
 +----------------------------------------+
 | Grupo:                                 |
 | - Rui Ventura,      ist id: 81045      |
 | - Goncalo Baptista, ist id: 81196      |
 | - Sara Azinhal,     ist id: 81700      |
 +----------------------------------------+
*/

/*
 ------------------------------------------
 --- HEADERS, PROTOTYPES & DEFINITIONS ----
 ------------------------------------------
*/

/* -- Header files -- */

/* - C/POSIX library headers */
#include <sys/types.h> /* Data types */
#include <sys/stat.h> /* Data returned by the stat() function */
#include <sys/wait.h> /* Declarations for waiting */

#include <errno.h> /* System error numbers */
#include <fcntl.h> /* File control options */
#include <pthread.h> /* Threads (& mutexes, cond_vars) */
#include <signal.h> /* Signals */
#include <stdbool.h> /* Boolean type and values */
#include <stdio.h> /* Standard buffered I/O */
#include <stdlib.h> /* Standard library functions */
#include <string.h> /* String operations */
#include <time.h> /* Time types */
#include <unistd.h> /* Standard symbolic constants and types */

/* - Local headers */
#include "commandlinereader.h" /* see commandlinereader.h */
#include "list.h" /* see list.h */
#include "pidlist.h" /* see pidlist.h */
#include "shared.h" /* see shared.h */
#include "wrappers.h" /* see wrappers.h */

/* -- Definitions -- */

/* - Constants / Symbols */
/* Numerical type */
#define MAXPAR 4 /* Int: Maximum number of parallel processes */
#define LINE_LOGFILE_MAX 64 /* Int: Log file line's buffer size */

/* Character / String type */
#define PATH_LOGFILE_STR "log.txt" /* String: Path to the log file */
#define PATH_FIFOSHELL_STR "/tmp/par-shell-in" /* String: Path to FIFO */
#define PATH_CHILDOUT_STR "par-shell-out" /* String: Parcial path to child out */

/* -- Global variables -- */

/* Collection for child processes */
pidlist_t *g_lst_children;

/* Collection*/
list_t *g_lst_terminals;

/* Monitor thread */
pthread_t g_monitor_pt;

/* Single mutex associated with the globally shared resources */
pthread_mutex_t g_mutex;

/* Condition variables for the globally shared resources */
/* Used to limit parallelism */
pthread_cond_t g_child_cv;

/* Used to tip off moitoring thread */
pthread_cond_t g_monitoring_cv;

/* Number of children iterations (read from file, 0 by default) */
int g_iterations = -1;

/* Sum of the children processes' exec time */
int g_total_time = 0;

/* Number of child processes currently running */
int g_num_children = 0;

/* Determines if child process monitoring is required */
bool g_monitoring = true;

/* Logging file file pointer */
FILE *g_log_file;

/* -- Helper function prototypes */
/* Child process monitoring function */
void *process_monitor();

/* Allocated shared resources destroyer */
void destroySharedResources();

/* Clean up after being done */
void cleanup();

/* -- Signal handler prototypes */
/* Termination signal (SIGINT) handler / par-shell exit routine */
void handle_SIGINT(int signum);

/*
 -----------------------------------------------------------------------------
 ------------------------------- MAIN PROGRAM --------------------------------
 -----------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
  int psin; /* Par-shell-in fifo file descriptor */
  pid_t child_pid; /* child process id */
  
  char *args[CMD_ARGS_MAX+1], buffer_cmd[LINE_COMMAND_MAX]; /* cmd related */
  char buffer_log[LINE_LOGFILE_MAX]; /* log related */
  
  {
    s_signal(SIGINT, handle_SIGINT); /* SIGINT handler / exit routine */
    
    if((g_lst_children = lst_new()) == NULL) {
      fprintf(stderr, "lst_new: couldn't create list\n");
      exit(EXIT_FAILURE);
    } /* Child list */
    
    if((g_lst_terminals = newList()) == NULL) {
      fprintf(stderr, "newList: couldn't create list\n");
      exit(EXIT_FAILURE);
    } /* Terminals list */
    
    if((g_log_file = fopen(PATH_LOGFILE_STR, "a+")) == NULL) {
      handle_error("fopen");
    } /* Log file */
  
    while(!feof(g_log_file)) {
      fgets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* NULL or iteracao # */
      if(feof(g_log_file)) break;
      sscanf(buffer_log, "%*s %d", &g_iterations);
      
      fgets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* PID: # time: # s */
      fgets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* total time: # s */
      sscanf(buffer_log, "%*[^0-9] %d", &g_total_time);
    } /* Read logfile */
    clearerr(g_log_file); /* Clears errno set by feof */
    ++g_iterations;
    
    cond_init(&g_child_cv); /* Children limitation */
    cond_init(&g_monitoring_cv); /* Children control */
    mutex_init(&g_mutex); /* Resource 'keeper' (as in gate-keeper) */
    pcreate(&g_monitor_pt, process_monitor, NULL); /* Monitor thread */
    
    unlink(PATH_FIFOSHELL_STR); /* Delete fifo, if it exists */
    mk_fifo(PATH_FIFOSHELL_STR, S_IRWXU | S_IRWXG | S_IRWXO); /* rwxrwxrwx */
    
    /* input redirection (fifo -> stdin) */
    psin = fd_open(PATH_FIFOSHELL_STR, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    dup_2(psin, STDIN_FILENO);
    fd_close(psin);
  } /* Setting up */
  
  while(true) {
    int numargs = readLineArguments(args,
      CMD_ARGS_MAX + 1, buffer_cmd, LINE_COMMAND_MAX);
    
    if(args[0] == NULL /* || numargs <= 0 */) continue;
    if(numargs > 0) {
      if(!strcmp(args[0], COMMAND_SIGNIN_STR)) {
        pid_t tpid = (pid_t)atol(args[1]);
        if(addItem(g_lst_terminals, (long)tpid) != 0) {
          fprintf(stderr, "addItem: couldn't register terminal. Continuing..");
          kill(tpid, SIGUSR1);
        } else {
          printf("Terminal w/ PID %lu connected\n", (long)tpid);
        }
      } /* Terminal connected */
      else if(!strcmp(args[0], COMMAND_EXIT_STR)) {
        handle_SIGINT(EXIT_SUCCESS);
      } /* Par-shell exit routine */
      else if(!strcmp(args[0], COMMAND_STATS_STR)) {
        mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; /* 0755 */
        char buffer[LINE_COMMAND_MAX];
        int fifo = fd_open(args[1], O_WRONLY, mode);
        
        sprintf(buffer, "Processes running: %d; Total time: %02d s\n",
          g_num_children, g_total_time);
        fd_write(fifo, buffer, strlen(buffer));
        fd_close(fifo);
      } /* Terminal issued stats */
      else if(!strcmp(args[0], COMMAND_SIGNOFF_STR)) {
        pid_t tpid = (pid_t)atol(args[1]);
        if(removeItem(g_lst_terminals, (long)tpid) != 0) {
          fprintf(stderr, "removeItem: couldn't remove terminal.");
          kill(0, SIGINT);
        }
        
        printf("Terminal w/ PID %lu disconnected\n", (long)tpid);
      } /* Terminal disconnected */
      else {
        FILE *fp;
        if ((fp = fopen(args[0], "r")) == NULL) {
          perror(args[0]);
        }
        else {
          fclose(fp);
          
          mutex_lock(&g_mutex);
          while(g_num_children == MAXPAR) {
            cond_wait(&g_child_cv, &g_mutex);
          }
          mutex_unlock(&g_mutex);
          
          if((child_pid = fork()) < 0) {
            perror("fork");
          }
          else if(child_pid == 0) {
            int error, child_out; /* Child output fd */
            char buffer[32]; /* Concatenate output path */
            
            sprintf(buffer, "%s-%d%s", PATH_CHILDOUT_STR, getpid(), ".txt");
            child_out = fd_open(buffer, O_CREAT | O_WRONLY, 
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* rw-r--r-- */
            dup_2(child_out, STDOUT_FILENO);
            fd_close(child_out);
            
            execv(args[0], args);
            
            error = errno; /* Capture this after a possible failure */
            destroySharedResources();
            errno = error; /* Regain, so perror can report adequately */
            handle_error("execv");
          } /* Child process */
          else {
            mutex_lock(&g_mutex);
            if(insert_new_process(g_lst_children, child_pid, time(NULL)) != 0) {
              fprintf(stderr,
                "insert_new_process: failed to insert new process\n");
              exit(EXIT_FAILURE);
            }
            ++g_num_children;
            
            cond_signal(&g_monitoring_cv);
            mutex_unlock(&g_mutex);
          } /* Parent process */
        }
      } /* Executes a program */
    }
  }
;}

/*
 -----------------------------------------------------------------------------
 ------------------------- FUNCTION IMPLEMENTATIONS --------------------------
 -----------------------------------------------------------------------------
*/

/* -- Helper functions */
/* Child process monitoring function */
void *process_monitor() {
  pid_t child_pid; /* pid of zombie child */
  int status, time_interval; /* child exit status; interval between time_t's */
  
  while(true) {
    mutex_lock(&g_mutex);
    if(g_num_children > 0) {
      mutex_unlock(&g_mutex);
      
      child_pid = wait(&status);
      
      if(child_pid < 0) {
          if(errno == EINTR) /* Waiting interrupted */
            continue;
          else
            handle_error("wait");
      }
      
      mutex_lock(&g_mutex);
      time_interval = update_terminated_process(g_lst_children,
        child_pid, status, time(NULL));
      
      if(time_interval < 0) { /* Shouldn't happen.. Just in case */
        fprintf(stderr, "update_terminated_process: interval is negative\n");
        fprintf(stderr, "Not logging process %d..\n", child_pid);
      } else {
        g_total_time += time_interval;
  
        fprintf(g_log_file, "iteracao %d\n" \
                 "PID: %d execution time: %02d s\n" \
                 "total execution time: %02d s\n",
                 g_iterations++, child_pid, time_interval, g_total_time);
        fflush(g_log_file);
      }
      --g_num_children;
      
      cond_signal(&g_child_cv);
      mutex_unlock(&g_mutex);
      
    } else if(g_monitoring) {
      while(g_num_children == 0 && g_monitoring)
        cond_wait(&g_monitoring_cv, &g_mutex);
      mutex_unlock(&g_mutex);
    }
    else break;
  }
  mutex_unlock(&g_mutex);
  
  return NULL;
}

/* Allocated shared resources destroyer */
void destroySharedResources() {
  destroyList(g_lst_terminals);
  lst_destroy(g_lst_children);
  
  mutex_destroy(&g_mutex);
  
  cond_destroy(&g_child_cv);
  cond_destroy(&g_monitoring_cv);
}

/* Clean up after being done */
void cleanup() {
  mutex_lock(&g_mutex);
  g_monitoring = false;
  cond_signal(&g_monitoring_cv);
  mutex_unlock(&g_mutex);
  
  pjoin(g_monitor_pt);
  
  lst_print(g_lst_children);
  
  destroySharedResources();
  fclose(g_log_file);
}

/* -- Signal handlers */
/* Handle a signal that interrupts this process */
void handle_SIGINT(int signum) {
  node_t *node = g_lst_terminals->first;
  
  fd_close(STDIN_FILENO);
  unlink(PATH_FIFOSHELL_STR); /* So following terminals won't connect */
  
  while(node) {
    item_t item = node->item;
    kill(item, SIGUSR2);
    fprintf(stderr, "Sending a signal to terminal w/ pid %lu.\n", item);
    node = node->next;
  }
  
  cleanup();
  exit(signum);
}
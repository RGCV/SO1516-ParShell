/*
 +----------------------------------------+
 | Projecto - Aplicacao Par-Shell         |
 | Sistemas Operativos, LEIC-A, Grupo 38  |
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
#include <stdlib.h> /* Standard library functions */
#include <stdbool.h> /* Boolean type and values */
#include <stdio.h> /* Standard buffered I/O */
#include <string.h> /* String operations */
#include <stdarg.h> /* Handle variable argument list */
#include <errno.h> /* System error numbers */
#include <sys/wait.h> /* Declarations for waiting */
#include <sys/types.h> /* Data types */
#include <unistd.h> /* Standard symbolic constants and types */
#include <time.h> /* Time types */
#include <pthread.h> /* Threads (& mutexes, cond_vars)*/

/* - Local headers */
#include "commandlinereader.h" /* see commandlinereader.h */
#include "list.h" /* see list.h */

/* -- Definitions -- */
/* - Pre-processor macros */
/* Error handling */
#define handle_error(msg) { perror(msg); exit(EXIT_FAILURE); }

/* - Constants/Symbols */
/* Numerical type */
#define MAXPAR 4 /* Int: Maximum number of parallel processes */
#define CMD_ARGS_MAX 6 /* Int: Maximum number of command arguments */
#define LINE_COMMAND_MAX 128 /* Int: Command line's buffer size */
#define LINE_LOGFILE_MAX 64 /* Int: Log file line's buffer size */

/* Character type */
#define COMMAND_EXIT_STR "exit" /* String: Exit command */
#define PATH_LOGFILE_STR "log.txt" /* String Path to the log file */

/* -- Global variables -- */
/* Collection for child processes */
list_t *g_lst_children;

/* Single mutex associated with the globally shared resources */
pthread_mutex_t g_mutex;

/* Condition variables for the globally shared resources */
pthread_cond_t g_child_cv, g_monitoring_cv;

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
void destroySharedResources();

/* -- Wrapper function prototypes -- */
/* - stdio.h library */
/* I/O & File system */
FILE *f_open(const char *path, const char *mode);
void f_gets(char *s, int size, FILE *stream);
void f_flush(FILE *stream);
void f_close(FILE *stream);

/* - pthread.h library */
/* Threads */
void pcreate(pthread_t *thread, void* (*start_routine)(void*), void *arg);
void pjoin(pthread_t thread);

/* Mutexes */
void mutex_init(pthread_mutex_t *mutex);
void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void mutex_destroy(pthread_mutex_t *mutex);

/* Condition Variables */
void cond_init(pthread_cond_t *cv);
void cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex);
void cond_signal(pthread_cond_t *cv);
void cond_destroy(pthread_cond_t *cv);


/*
 -----------------------------------------------------------------------------
 ------------------------------- MAIN PROGRAM --------------------------------
 -----------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
  pid_t child_pid; /* child process id */
  pthread_t monitor_thread; /* monitor thread */
  char *args[CMD_ARGS_MAX+1], buffer_cmd[LINE_COMMAND_MAX]; /* cmd related */
  char buffer_log[LINE_LOGFILE_MAX]; /* log related */

  if((g_lst_children = lst_new()) == NULL) {
    if(fprintf(stderr, "lst_new: couldn't create list\n") < 0)
      handle_error("fprintf");
    exit(EXIT_FAILURE);
  }

  g_log_file = f_open(PATH_LOGFILE_STR, "a+");

  while(!feof(g_log_file)) {
    f_gets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* NULL or iteracao # */
    if(feof(g_log_file)) break;
    if(sscanf(buffer_log, "%*s %d", &g_iterations) != 1)
      handle_error("sscanf");
    
    f_gets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* PID: # time: # s */
    f_gets(buffer_log, LINE_LOGFILE_MAX, g_log_file); /* total time: # s */
    if(sscanf(buffer_log, "%*[^0-9] %d", &g_total_time) != 1)
      handle_error("sscanf");
  }
  
  ++g_iterations;
  
  cond_init(&g_child_cv);
  cond_init(&g_monitoring_cv);
  mutex_init(&g_mutex);
  pcreate(&monitor_thread, process_monitor, NULL);
  
  while(true) {
    int numargs = readLineArguments(args,
      CMD_ARGS_MAX + 1, buffer_cmd, LINE_COMMAND_MAX);
    
    if(args[0] == NULL) continue;
    if(numargs < 0 || (numargs > 0 && !strcmp(args[0], COMMAND_EXIT_STR))) {
      
      mutex_lock(&g_mutex);
      g_monitoring = false;
      cond_signal(&g_monitoring_cv);
      mutex_unlock(&g_mutex);
      
      pjoin(monitor_thread);
      
      lst_print(g_lst_children);
      
      destroySharedResources();
      f_close(g_log_file);
      return EXIT_SUCCESS;
    }
    else {
      FILE *fp; /* To check file existance */
      if ((fp = fopen(args[0], "r")) == NULL)
        perror(args[0]);
      else {
        f_close(fp);
        
        mutex_lock(&g_mutex);
        while(g_num_children == MAXPAR)
          cond_wait(&g_child_cv, &g_mutex);
        mutex_unlock(&g_mutex);
        
        if((child_pid = fork()) < 0) perror("fork");
        else if(child_pid == 0) {
          execv(args[0],args);
          
          destroySharedResources();
          handle_error("execv");
        }
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
        }
      }
    }
  }
}


/*
 -----------------------------------------------------------------------------
 ------------------------- FUNCTION IMPLEMENTATIONS --------------------------
 -----------------------------------------------------------------------------
*/

/* -- Helper function prototypes */

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
      
      if(time_interval < 0) {
        fprintf(stderr, "update_terminated_process: interval is negative\n");
        fprintf(stderr, "Not logging process %d..\n", child_pid);
      } else {
        g_total_time += time_interval;
  
        fprintf(g_log_file, "iteracao %d\n" \
                "PID: %d execution time: %02d s\n" \
                "total execution time: %02d s\n",
                g_iterations++, child_pid, time_interval, g_total_time);
        f_flush(g_log_file);
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

/* Destroy shared resources */
void destroySharedResources() {
  lst_destroy(g_lst_children);
  
  mutex_destroy(&g_mutex);
  
  cond_destroy(&g_child_cv);
  cond_destroy(&g_monitoring_cv);
}

/* -- Wrapper functions -- */

/* I/O & File system */
FILE *f_open(const char *path, const char *mode) {
  FILE *fp;
  if((fp = fopen(path, mode)) == NULL)
    handle_error("f_gets: fgets");
  return fp;
}

void f_gets(char *s, int size, FILE *stream) {
  if(fgets(s, size, stream) == NULL && errno != EXIT_SUCCESS)
    handle_error("f_gets: fgets");
}

void f_flush(FILE *stream) {
  if(fflush(stream) == EOF)
    handle_error("f_flush: fflush");
}

void f_close(FILE *stream) {
  if(fclose(stream) == EOF)
    handle_error("f_close: fclose");
}

/* PThreads */
void pcreate(pthread_t *thread, void* (*start_routine)(void*), void *arg) {
  if(pthread_create(thread, NULL, start_routine, arg) != 0)
    handle_error("pcreate: pthread_create");
}

void pjoin(pthread_t thread) {
  if(pthread_join(thread, NULL) != 0)
    handle_error("pjoin: pthread_join");
}

/* Mutexes */
void mutex_destroy(pthread_mutex_t *mutex) {
  if(pthread_mutex_destroy(mutex) != 0)
    handle_error("mutex_destroy: pthread_mutex_destroy");
}

void mutex_lock(pthread_mutex_t *mutex) {
  if(pthread_mutex_lock(mutex) != 0)
    handle_error("mutex_init: pthread_mutex_init");
}

void mutex_unlock(pthread_mutex_t *mutex) {
  if(pthread_mutex_unlock(mutex) != 0)
    handle_error("mutex_unlock: pthread_mutex_unlock");
}

void mutex_init(pthread_mutex_t *mutex) {
  if(pthread_mutex_init(mutex, NULL) != 0)
    handle_error("mutex_init: pthread_mutex_init");
}

/* Condition Variables */
void cond_init(pthread_cond_t *cv) {
  if(pthread_cond_init(cv, NULL) != 0)
    handle_error("cond_init: pthread_cond_init");
}

void cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex) {
  if(pthread_cond_wait(cv, mutex) != 0)
    handle_error("cond_wait: pthread_cond_wait");
}

void cond_signal(pthread_cond_t *cv) {
  if(pthread_cond_signal(cv) != 0)
    handle_error("cond_signal: pthread_cond_signal");
}

void cond_destroy(pthread_cond_t *cv) {
  if(pthread_cond_destroy(cv) != 0)
    handle_error("cond_destroy: pthread_cond_destroy");
}

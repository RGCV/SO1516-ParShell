/*
// Wrapper functions / handlers
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

/*
 ------------------------------------------
 ---------------- HEADERS -----------------
 ------------------------------------------
*/

/* - C/POSIX library headers */
#include <sys/stat.h> /* Data returned by the stat() function */
#include <sys/types.h> /* Data types */

#include <errno.h> /* System error numbers */
#include <fcntl.h> /* File control options */
#include <pthread.h> /* Threads (& mutexes, cond_vars) */
#include <signal.h> /* Signals */
#include <stdarg.h> /* Variable arguments */
#include <stdbool.h> /* Boolean type and values */
#include <stdlib.h> /* Standard library functions */
#include <unistd.h> /* Standard symbolic constants and types */

/* - Local header files */
#include "shared.h" /* see shared.h */
#include "wrappers.h" /* see wrappers.h */

/*
 -----------------------------------------------------------------------------
 ------------------------- FUNCTION IMPLEMENTATIONS --------------------------
 -----------------------------------------------------------------------------
*/

/* -- Wrapper functions -- */
/* Signals */
void s_signal(int signo, void (*func)(int)) {
  if(signal(signo, func) == SIG_ERR)
    handle_error("s_signal: signal");
}

/* I/O & File system (sys calls) */
int dup_2(int fildes, int fildes2) {
  int fd;
  
  if((fd = dup2(fildes, fildes2)) < 0)
    handle_error("dup_2: dup2");
  return fd;
}

int fd_open(const char *pathname, int flags, mode_t mode) {
  int fd;
  
  if((fd = open(pathname, flags, mode)) < 0)
    handle_error("fd_open: open");
  return fd;
}

ssize_t fd_read(int fildes, void *buf, size_t nbytes) {
  int n;
  
  if((n = read(fildes, buf, nbytes)) < 0 && errno != EINTR)
    handle_error("fd_read: read");
  return n;
}

void fd_write(int fildes, void *buf, size_t nbytes) {
  if(write(fildes, buf, nbytes) < 0 && errno != EINTR)
    handle_error("fd_write: write");
}

void fd_close(int fildes) {
  if(close(fildes) < 0 && errno != EINTR)
    handle_error("fd_close: close");
}

void mk_fifo(const char *pathname, mode_t mode) {
  if(mkfifo(pathname, mode) != 0)
    handle_error("mk_fifo: mkfifo");
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

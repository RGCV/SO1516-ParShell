/*
// Wrapper functions / handlers (header file)
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

/*
 ------------------------------------------
 --- HEADERS, PROTOTYPES & DEFINITIONS ----
 ------------------------------------------
*/

/* -- Header files -- */

/* - C/POSIX library headers */
#include <sys/stat.h> /* Data returned by the stat() function */
#include <sys/types.h> /* Data types */

#include <stdio.h> /* Standard buffered I/O */
#include <pthread.h> /* Threads (& mutexes, cond_vars) */

/* -- Wrapper function prototypes -- */
/* - signal.h */
void s_signal(int signo, void (*func)(int));

/* - fcntl.h / sys/stat.h */
/* I/O & File system (sys calls) */
int dup_2(int fildes, int fildes2);
int fd_open(const char *pathname, int flags,mode_t mode);
ssize_t fd_read(int fildes, void *buf, size_t nbytes);
void fd_write(int fildes, void *buf, size_t nbytes);
void fd_close(int fildes);
void mk_fifo(const char *pathname, mode_t mode);

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

#endif /* __WRAPPERS_H__ */
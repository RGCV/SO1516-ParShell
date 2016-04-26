/*
// Parallel Shell Terminal
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

/*
 ------------------------------------------
 --- HEADERS, PROTOTYPES & DEFINITIONS ----
 ------------------------------------------
*/

/* - C/POSIX library headers */
#include <sys/stat.h> /* Data returned by the stat() function */

#include <fcntl.h> /* File control options */
#include <signal.h> /* Signals */
#include <stdbool.h> /* Boolean type and values */
#include <stdio.h> /* Standard buffered I/O */
#include <stdlib.h> /* Standard library functions */
#include <string.h> /* String operations */
#include <unistd.h> /* Standard symbolic constants and types */

#include "commandlinereader.h" /* see commandlinereader.h */
#include "shared.h" /* see shared.h */
#include "wrappers.h" /* see wrappers.h */

/* -- Global variables -- */
/* The output fifo fd */
int g_fifo_out_fd;

/* -- Helper function prototypes */
/* Clean up after being done */
void cleanup();

/* -- Signal handler prototypes */
/* Handles a signal sent by par-shell that it couldn't register the terminal */
void handle_SIGUSR1(int signum);

/* Handles a signal sent by par-shell that issues terminal termination */
void handle_SIGUSR2(int signum);

/* Handles any interrupt signal issued to the terminal */
void handle_SIGINT(int signum);

/* Handles a signal sent when the terminal writes to a broken pipe */
void handle_SIGPIPE(int signum);

/*
 -----------------------------------------------------------------------------
 ------------------------------- MAIN PROGRAM --------------------------------
 -----------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
  char *args[CMD_ARGS_MAX+1], buffer_cmd[LINE_COMMAND_MAX];
  
  if(argc != 2) {
    fprintf(stderr, "Incorrect arguments: par-shell-terminal <pipe name>\n");
    exit(EXIT_FAILURE);
  }
  
  /* Assign signal handlers */
  s_signal(SIGUSR1, handle_SIGUSR1); /* Couldn't register terminal */
  s_signal(SIGUSR2, handle_SIGUSR2); /* Par-shell signaled to exit */
  s_signal(SIGINT, handle_SIGINT); /* In case it gets interrupted */
  s_signal(SIGPIPE, handle_SIGPIPE); /* Broken pipe */
  
  g_fifo_out_fd = fd_open(argv[1], O_WRONLY,
    S_IRWXU | S_IRWXG | S_IRWXO); /* rwxrwxrwx */
  
  /* Initial terminal -> par-shell communcation */
  sprintf(buffer_cmd, "%s %d\n", COMMAND_SIGNIN_STR, getpid());
  fd_write(g_fifo_out_fd, buffer_cmd, strlen(buffer_cmd));
  
  while(true) {
    char buffer[LINE_COMMAND_MAX];
    int numargs = readLineArguments(args,
      CMD_ARGS_MAX + 1, buffer_cmd, LINE_COMMAND_MAX);
    
    if(args[0] == NULL /* || numargs <= 0 */) continue;
    if(numargs > 0) {
      if(!strcmp(args[0], COMMAND_EXIT_STR)) {
        cleanup();
        exit(EXIT_SUCCESS);
      } /* Exit command */
      else if(!strcmp(args[0], COMMAND_STATS_STR)) {
        char fifoName[32];
        int fifo; /* Temp fifo par-shell uses to send this terminal info */
        mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; /* rwxrwxrwx */
        
        /* Create a temporary fifo */
        sprintf(fifoName, "%s-%d", PATH_FIFOTERMINAL_STR, getpid());
        mk_fifo(fifoName, mode);
        sprintf(buffer, "%s %s\n", args[0], fifoName);
        fd_write(g_fifo_out_fd, buffer, strlen(buffer));
        
        /* Read the response, when it is made available */
        printf("Waiting for stats -------\n");
        fifo = fd_open(fifoName, O_RDONLY, mode);
        fd_read(fifo, buffer, sizeof(buffer) / sizeof(char));
        fd_close(fifo);
        printf("Stats Received ----------\n%s", buffer);
        
        /* Delete the temp fifo */
        unlink(fifoName);
      } /* Stats command */
      else if(!strcmp(args[0], COMMAND_EXITG_STR)) {
        sprintf(buffer, "%s\n", COMMAND_EXIT_STR);
        fd_write(g_fifo_out_fd, buffer, strlen(buffer));
      } /* Exit-global command */
      else {
        /* Rebuilds the command */
        int i = 1;
        
        sprintf(buffer, "%s", args[0]);
        while(args[i]) {
          strcat(buffer, " ");
          strcat(buffer, args[i++]);
        }
        strcat(buffer, "\n");
        fd_write(g_fifo_out_fd, buffer, strlen(buffer));
      } /* Any other command */
    }
  }
  
  fd_close(g_fifo_out_fd);
  return EXIT_SUCCESS;
}

/*
 -----------------------------------------------------------------------------
 ------------------------- FUNCTION IMPLEMENTATIONS --------------------------
 -----------------------------------------------------------------------------
*/

/* -- Helper functions */
/* Clean up after being done */
void cleanup() {
  char buffer[LINE_COMMAND_MAX];

  sprintf(buffer, "%s %d\n", COMMAND_SIGNOFF_STR, getpid());
  fd_write(g_fifo_out_fd, buffer, strlen(buffer));
  
  fd_close(g_fifo_out_fd);
}

/* -- Signal handlers */
/* Handles a signal sent by par-shell that it couldn't register the terminal */
void handle_SIGUSR1(int signum) {
  fprintf(stderr, "par-shell couldn't register this terminal. Exiting..\n");
  fd_close(g_fifo_out_fd);
  exit(signum);
}

/* Handles a signal sent by par-shell to close down */
void handle_SIGUSR2(int signum) {
  fprintf(stderr, "par-shell signaled to exit. Exiting..\n");
  fd_close(g_fifo_out_fd);
  exit(signum);
}

/* Handles a signal sent when the terminal writes to a broken pipe */
void handle_SIGPIPE(int signum) {
  fprintf(stderr, "Received SIGPIPE: Broken fifo. Exiting..\n");
  exit(signum);
}

/* Handles interrupt signal sent to terminal */
void handle_SIGINT(int signum) {
  fprintf(stderr, "Received SIGINT: Exiting, warning par-shell!\n");
  cleanup();
  exit(signum);
}
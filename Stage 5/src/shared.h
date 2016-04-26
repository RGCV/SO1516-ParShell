/*
// Shared definitions, macros & others (header file)
// Sistemas Operativos - LEIC-A, Grupo 38
// IST/ULisboa 2015-16
*/

#ifndef __SHARED_H__
#define __SHARED_H__

/*
 ------------------------------------------
 -------------- DEFINITIONS ---------------
 ------------------------------------------
*/

/* -- Definitions -- */

/* - Pre-processor macros */
/* Error handling */
#define handle_error(msg) { \
    perror(msg); \
    exit(EXIT_FAILURE); \
}

/* - Constants/Symbols */
/* Numerical type */
#define CMD_ARGS_MAX 6 /* Int: Maximum number of command arguments */
#define LINE_COMMAND_MAX 128 /* Int: Command line's buffer size */

/* Character / String type */
#define PATH_FIFOTERMINAL_STR "/tmp/par-shell-terminal" /* String: Name*/
#define COMMAND_SIGNIN_STR "signin" /* String: Terminal signing in command */
#define COMMAND_SIGNOFF_STR "signoff" /* String: Terminal signing off command */
#define COMMAND_EXIT_STR "exit" /* String: Exit command */
#define COMMAND_EXITG_STR "exit-global" /* String: Exit command */
#define COMMAND_STATS_STR "stats" /* String: Stats command */

#endif /* __SHARED_H__ */

#ifndef SHELL_H
#define SHELL_H

#include <readline/readline.h>
#include <readline/history.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>   // for open()

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20


// =====================
// Background Jobs Support
// =====================
#define MAX_JOBS 50

typedef struct {
    pid_t pid;
    char command[256];
    int active;
} Job;

extern Job jobs[MAX_JOBS];

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **arglist);
void add_to_history(const char* cmdline);
void show_history();
char* get_history_command(int n);\
int execute_with_redirection(char** arglist);
int execute_with_pipe(char** left_cmd, char** right_cmd);
int execute_background(char** arglist);
void reap_zombie_processes();
int handle_if_condition(char *cmdline);
int handle_for_loop(char *cmdline);
int handle_while_loop(char *cmdline);



// Global history variables
extern char* history[HISTORY_SIZE];
extern int history_count;

#endif // SHELL_H

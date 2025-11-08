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

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **arglist);
void add_to_history(const char* cmdline);
void show_history();
char* get_history_command(int n);

// Global history variables
extern char* history[HISTORY_SIZE];
extern int history_count;

#endif // SHELL_H

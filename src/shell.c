#include "shell.h"

char* history[HISTORY_SIZE];
int history_count = 0;

// =========================
// FEATURE 4: Readline Integration
// =========================
char* read_cmd(char* prompt) {
    char* cmdline = readline(prompt);

    if (cmdline == NULL) // Ctrl+D
        return NULL;

    if (strlen(cmdline) > 0)
        add_history(cmdline);  // Readline’s built-in history

    return cmdline;
}

// =========================
// Tokenization Logic
// =========================
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        // skip whitespace
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        // handle single-character operators
        if (*cp == '>' || *cp == '<' || *cp == '|') {
            arglist[argnum][0] = *cp;
            arglist[argnum][1] = '\0';
            argnum++;
            cp++;
            continue;
        }

        // handle regular words
        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t' || *cp == '>' || *cp == '<' || *cp == '|'))
            len++;

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++)
            free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}


// =========================
// Built-in Command Handler
// =========================
int handle_builtin(char **arglist) {
    if (arglist[0] == NULL)
        return 1; // empty input handled

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(arglist[1]) != 0) {
            perror("cd");
        }
        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <directory>\n");
        printf("  exit\n");
        printf("  help\n");
        printf("  jobs\n");
        printf("  history\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        show_history();
        return 1;
    }

    return 0; // not a built-in command
}

// =========================
// Command History Functions
// =========================
void add_to_history(const char* cmdline) {
    if (cmdline == NULL || strlen(cmdline) == 0) return;

    // Free oldest if full
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history[i - 1] = history[i];
        history_count--;
    }

    history[history_count] = strdup(cmdline);
    history_count++;
}

void show_history() {
    for (int i = 0; i < history_count; i++)
        printf("%d %s\n", i + 1, history[i]);
}

char* get_history_command(int n) {
    if (n < 1 || n > history_count) {
        fprintf(stderr, "No such command in history: !%d\n", n);
        return NULL;
    }
    return strdup(history[n - 1]);
}

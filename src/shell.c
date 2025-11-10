#include <signal.h>
#include "shell.h"

extern Job jobs[MAX_JOBS];
char* history[HISTORY_SIZE];
int history_count = 0;

// =========================
// FEATURE 4: Readline Integration
// =========================
char* read_cmd(char* prompt, FILE* fp) {
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
        printf("Active background jobs:\n");
        int found = 0;
        for (int i = 0; i < MAX_JOBS; i++) {
            extern Job jobs[];
            if (jobs[i].active) {
                printf("[%d] PID: %d  CMD: %s\n", i + 1, jobs[i].pid, jobs[i].command);
                found = 1;
            }
        }
        if (!found) printf("(none)\n");
        return 1;
    }


    if (strcmp(arglist[0], "history") == 0) {
        show_history();
        return 1;
    }

    if (strcmp(arglist[0], "fg") == 0) {
        fg_command();
        return 1;
    }

    if (strcmp(arglist[0], "bg") == 0) {
        bg_command();
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

int handle_if_condition(char *cmdline) {
    // Detect start of an if block
    if (strncmp(cmdline, "if ", 3) != 0)
        return 0;  // not an if statement

    // Extract the condition command (after 'if ')
    char if_cmd[256] = {0};
    strncpy(if_cmd, cmdline + 3, sizeof(if_cmd) - 1);

    char then_block[512] = {0};
    char else_block[512] = {0};
    int in_then = 0, in_else = 0;
    char line[MAX_LEN];

    // Keep reading until "fi" is found
    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        // Strip newline
        line[strcspn(line, "\n")] = '\0';

        // Detect keywords
        if (strcmp(line, "then") == 0) {
            in_then = 1;
            in_else = 0;
            continue;
        } else if (strcmp(line, "else") == 0) {
            in_else = 1;
            in_then = 0;
            continue;
        } else if (strcmp(line, "fi") == 0) {
            break;
        }

        // Append to the right block
        if (in_then) {
            strcat(then_block, line);
            strcat(then_block, "\n");
        } else if (in_else) {
            strcat(else_block, line);
            strcat(else_block, "\n");
        }
    }

    // Execute the 'if' condition command
    char **if_args = tokenize(if_cmd);
    if (if_args == NULL) return 1;

    pid_t pid = fork();
    int status;
    if (pid == 0) {
        execvp(if_args[0], if_args);
        perror("exec failed");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
    }

    for (int i = 0; if_args[i] != NULL; i++) free(if_args[i]);
    free(if_args);

    int success = (WEXITSTATUS(status) == 0);
    char *chosen_block = success ? then_block : else_block;

    if (strlen(chosen_block) == 0)
        return 1; // nothing to run

    // Execute chosen block
    char **branch_args = tokenize(chosen_block);
    if (branch_args == NULL) return 1;

    if (!handle_builtin(branch_args))
        execute(branch_args);

    for (int i = 0; branch_args[i] != NULL; i++) free(branch_args[i]);
    free(branch_args);

    return 1;
}

int handle_for_loop(char *cmdline) {
    if (strncmp(cmdline, "for ", 4) != 0)
        return 0; // not a for loop

    // Find structure keywords
    char *do_part = strstr(cmdline, "do");
    char *done_part = strstr(cmdline, "done");

    if (!do_part || !done_part) {
        fprintf(stderr, "Syntax error: missing 'do' or 'done'\n");
        return 1;
    }

    // Extract "for var in ..."
    char header[256] = {0};
    strncpy(header, cmdline + 4, do_part - (cmdline + 4));
    header[do_part - (cmdline + 4)] = '\0';

    char var[32] = {0};
    char items[256] = {0};

    // Parse variable and items
    if (sscanf(header, "%s in %[^\n]", var, items) != 2) {
        fprintf(stderr, "Syntax error in for-loop header\n");
        return 1;
    }

    // Extract command inside do...done
    char body[256] = {0};
    strncpy(body, do_part + 2, done_part - (do_part + 2));
    body[done_part - (do_part + 2)] = '\0';

    // Tokenize items
    char *token = strtok(items, " ");
    while (token != NULL) {
        // Build the body command replacing $var
        char expanded[512];
        char *pos = strstr(body, "$");
        if (pos) {
            snprintf(expanded, sizeof(expanded), "%.*s%s%s",
                     (int)(pos - body), body, token, pos + strlen(var) + 1);
        } else {
            strncpy(expanded, body, sizeof(expanded));
        }

        // Execute the expanded body
        char **args = tokenize(expanded);
        if (args != NULL) {
            if (!handle_builtin(args))
                execute(args);
            for (int i = 0; args[i] != NULL; i++)
                free(args[i]);
            free(args);
        }

        token = strtok(NULL, " ");
    }

    return 1; // handled
}

int handle_while_loop(char *cmdline) {
    if (strncmp(cmdline, "while ", 6) != 0)
        return 0; // not a while loop

    char *do_part = strstr(cmdline, "do");
    char *done_part = strstr(cmdline, "done");

    if (!do_part || !done_part) {
        fprintf(stderr, "Syntax error: missing 'do' or 'done'\n");
        return 1;
    }

    char condition[256] = {0};
    strncpy(condition, cmdline + 6, do_part - (cmdline + 6));
    condition[do_part - (cmdline + 6)] = '\0';

    char body[256] = {0};
    strncpy(body, do_part + 2, done_part - (do_part + 2));
    body[done_part - (do_part + 2)] = '\0';

    while (1) {
        // Evaluate condition
        char **cond_args = tokenize(condition);
        if (cond_args == NULL) break;

        pid_t pid = fork();
        int status;
        if (pid == 0) {
            execvp(cond_args[0], cond_args);
            exit(1);
        } else {
            waitpid(pid, &status, 0);
        }

        int success = (WEXITSTATUS(status) == 0);

        for (int i = 0; cond_args[i] != NULL; i++)
            free(cond_args[i]);
        free(cond_args);

        if (!success)
            break; // condition failed

        // Execute body
        char **body_args = tokenize(body);
        if (body_args != NULL) {
            if (!handle_builtin(body_args))
                execute(body_args);
            for (int i = 0; body_args[i] != NULL; i++)
                free(body_args[i]);
            free(body_args);
        }
    }

    return 1;
}

// Ignore Ctrl+C / Ctrl+Z in shell
void setup_signal_handlers() {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
}

// Custom SIGINT handler
void sigint_handler(int sig) {
    printf("\n(Type 'exit' to quit)\n");
    printf("%s", PROMPT);
    fflush(stdout);
}

// Custom SIGTSTP handler
void sigtstp_handler(int sig) {
    printf("\n(Suspension ignored in shell)\n");
    printf("%s", PROMPT);
    fflush(stdout);
}

// Bring last stopped job to foreground
void fg_command() {
    for (int i = MAX_JOBS - 1; i >= 0; i--) {
        if (jobs[i].active) {
            int status;
            printf("Resuming PID %d in foreground...\n", jobs[i].pid);
            kill(jobs[i].pid, SIGCONT);
            waitpid(jobs[i].pid, &status, 0);
            jobs[i].active = 0;
            return;
        }
    }
    printf("No background jobs.\n");
}

// Resume last stopped job in background
void bg_command() {
    for (int i = MAX_JOBS - 1; i >= 0; i--) {
        if (jobs[i].active) {
            printf("Resuming PID %d in background...\n", jobs[i].pid);
            kill(jobs[i].pid, SIGCONT);
            return;
        }
    }
    printf("No background jobs.\n");
}

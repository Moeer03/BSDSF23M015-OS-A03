#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
    // Check for !n re-execution
    if (cmdline[0] == '!' && strlen(cmdline) > 1) {
        int n = atoi(cmdline + 1);
        free(cmdline);
        cmdline = get_history_command(n);
        if (cmdline == NULL) continue;
        printf("%s\n", cmdline); // echo the command being executed
    }

    // Store command in history
    add_to_history(cmdline);

    // Tokenize & execute
    if ((arglist = tokenize(cmdline)) != NULL) {
        if (!handle_builtin(arglist))
            execute(arglist);

        for (int i = 0; arglist[i] != NULL; i++)
            free(arglist[i]);
        free(arglist);
    }

    free(cmdline);
    }
    printf("\nShell exited.\n");
    return 0;
}

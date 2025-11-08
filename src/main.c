#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // tokenize first
        arglist = tokenize(cmdline);
        if (arglist == NULL) {
            free(cmdline);
            continue;
        }

        // check for built-in
        if (!handle_builtin(arglist)) {
            execute(arglist);
        }

        // free allocated memory
        for (int i = 0; arglist[i] != NULL; i++) {
            free(arglist[i]);
        }
        free(arglist);
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

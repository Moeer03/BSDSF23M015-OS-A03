#include "shell.h"

int main() {
    setup_signal_handlers();
    char* cmdline;
    char** arglist;
    


    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // --- Handle IF condition ---
        if (strncmp(cmdline, "if ", 3) == 0) {
            handle_if_condition(cmdline);
            free(cmdline);
            continue;
        }

        // --- Handle FOR loop ---
        if (strncmp(cmdline, "for ", 4) == 0) {
            handle_for_loop(cmdline);
            free(cmdline);
            continue;
        }

        // --- Handle WHILE loop ---
        if (strncmp(cmdline, "while ", 6) == 0) {
            handle_while_loop(cmdline);
            free(cmdline);
            continue;
        }

        // --- Handle command chaining using ';' ---
        char* token = strtok(cmdline, ";");
        while (token != NULL) {
            char* sub_cmd = strdup(token);

            // Handle !n re-execution
            if (sub_cmd[0] == '!' && strlen(sub_cmd) > 1) {
                int n = atoi(sub_cmd + 1);
                free(sub_cmd);
                sub_cmd = get_history_command(n);
                if (sub_cmd == NULL) {
                    token = strtok(NULL, ";");
                    continue;
                }
                printf("%s\n", sub_cmd);
            }

            // Add command to history
            add_to_history(sub_cmd);

            // Tokenize and execute
            char** arglist = tokenize(sub_cmd);
            if (arglist != NULL) {
                if (!handle_builtin(arglist))
                    execute(arglist);

                for (int i = 0; arglist[i] != NULL; i++)
                    free(arglist[i]);
                free(arglist);
            }

            free(sub_cmd);
            token = strtok(NULL, ";");
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

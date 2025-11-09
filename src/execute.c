#include "shell.h"

int execute(char* arglist[]) {
    // Detect pipe position
    int pipe_pos = -1;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    // If there's a pipe
    if (pipe_pos != -1) {
        arglist[pipe_pos] = NULL; // split the command
        char** left_cmd = arglist;
        char** right_cmd = &arglist[pipe_pos + 1];
        return execute_with_pipe(left_cmd, right_cmd);
    }

    // Check for redirection
    int in_redirect = -1, out_redirect = -1;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) in_redirect = i;
        if (strcmp(arglist[i], ">") == 0) out_redirect = i;
    }

    if (in_redirect != -1 || out_redirect != -1)
        return execute_with_redirection(arglist);

    // Normal execution
    int status;
    int cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);

        case 0: // Child
            execvp(arglist[0], arglist);
            perror("Command not found");
            exit(1);

        default:
            waitpid(cpid, &status, 0);
            return 0;
    }
}

// ======================
// I/O Redirection
// ======================
int execute_with_redirection(char** arglist) {
    int in_fd, out_fd;
    int in_redirect = -1, out_redirect = -1;

    // Find positions
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) in_redirect = i;
        if (strcmp(arglist[i], ">") == 0) out_redirect = i;
    }

    int cpid = fork();
    if (cpid == 0) { // child
        if (in_redirect != -1) {
            in_fd = open(arglist[in_redirect + 1], O_RDONLY);
            if (in_fd < 0) {
                perror("input file");
                exit(1);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
            arglist[in_redirect] = NULL;
        }

        if (out_redirect != -1) {
            out_fd = open(arglist[out_redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("output file");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
            arglist[out_redirect] = NULL;
        }

        execvp(arglist[0], arglist);
        perror("exec failed");
        exit(1);
    } else if (cpid > 0) {
        waitpid(cpid, NULL, 0);
    } else {
        perror("fork failed");
    }

    return 0;
}

// ======================
// Pipe Execution
// ======================
int execute_with_pipe(char** left_cmd, char** right_cmd) {
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return -1;
    }

    pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(left_cmd[0], left_cmd);
        perror("exec left failed");
        exit(1);
    }

    pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(right_cmd[0], right_cmd);
        perror("exec right failed");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}

#include "shell.h"

Job jobs[MAX_JOBS]; // global job list

// ======================
// Job Management Helpers
// ======================
void add_job(pid_t pid, char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].pid = pid;
            strncpy(jobs[i].command, cmd, sizeof(jobs[i].command));
            jobs[i].active = 1;
            break;
        }
    }
}

void reap_zombie_processes() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active && jobs[i].pid == pid) {
                jobs[i].active = 0;
                printf("[+] Background job %d (%s) finished.\n", pid, jobs[i].command);
                break;
            }
        }
    }
}

// ======================
// Main execute()
// ======================
int execute(char* arglist[]) {
    reap_zombie_processes();

    // Background check
    int bg_flag = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "&") == 0) {
            bg_flag = 1;
            arglist[i] = NULL;
            break;
        }
    }

    // Pipe check
    int pipe_pos = -1;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }
    if (pipe_pos != -1) {
        arglist[pipe_pos] = NULL;
        char** left_cmd = arglist;
        char** right_cmd = &arglist[pipe_pos + 1];
        return execute_with_pipe(left_cmd, right_cmd);
    }

    // Redirection check
    int in_redirect = -1, out_redirect = -1;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) in_redirect = i;
        if (strcmp(arglist[i], ">") == 0) out_redirect = i;
    }
    if (in_redirect != -1 || out_redirect != -1)
        return execute_with_redirection(arglist);

    // Background process
    if (bg_flag)
        return execute_background(arglist);

    // Normal foreground
    int status;
    int cpid = fork();
    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
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

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) in_redirect = i;
        if (strcmp(arglist[i], ">") == 0) out_redirect = i;
    }

    pid_t cpid = fork();
    if (cpid == 0) {
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
            out_fd = open(arglist[out_redirect + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);
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

// ======================
// Background Execution
// ======================
int execute_background(char** arglist) {
    pid_t pid = fork();

    if (pid == 0) {
        setsid();
        execvp(arglist[0], arglist);
        perror("Background exec failed");
        exit(1);
    } else if (pid > 0) {
        printf("[+] Started background job PID %d\n", pid);
        char cmdline[256] = "";
        for (int i = 0; arglist[i] != NULL; i++) {
            strcat(cmdline, arglist[i]);
            strcat(cmdline, " ");
        }
        add_job(pid, cmdline);
        return 0;
    } else {
        perror("fork failed");
        return -1;
    }
}

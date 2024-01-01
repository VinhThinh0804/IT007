#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 80 
#define HISTORY_SIZE 10 

char history[HISTORY_SIZE][MAX_LINE];
int history_index = 0;
pid_t running_pid;

void add_to_history(const char *command) {
    strcpy(history[history_index], command);
    history_index = (history_index + 1) % HISTORY_SIZE;
}

void print_history() {
    int i = 0;
    int index = history_index;
    do {
        if (strlen(history[index]) > 0) {
            printf("%s\n", history[index]);
        }
        index = (index + 1) % HISTORY_SIZE;
        i++;
    } while (i < HISTORY_SIZE);
}

void signal_handler(int signo) {
    if (signo == SIGINT && running_pid > 0) {
        kill(running_pid, SIGINT);
    }
    else{
        printf("\n");
        exit(EXIT_SUCCESS);
    }
}

void execute_command(char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execvp(args[0], args);

        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        running_pid = pid; 
        int status;
        waitpid(pid, &status, 0); 
        running_pid = 0; 
    }
}

void execute_pipeline(char *args1[], char *args2[]) {
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        close(pipe_fd[0]); 

        dup2(pipe_fd[1], STDOUT_FILENO); 
        close(pipe_fd[1]); 

        execvp(args1[0], args1); 
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid1 > 0) {
        waitpid(pid1, NULL, 0);

        pid_t pid2 = fork();

        if (pid2 == 0) {
            close(pipe_fd[1]); 

            dup2(pipe_fd[0], STDIN_FILENO); 
            close(pipe_fd[0]);

            execvp(args2[0], args2); 
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid2 > 0) {
            close(pipe_fd[0]); 
            close(pipe_fd[1]); 

            waitpid(pid2, NULL, 0); 
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

void execute_with_redirection(char *args[], int input_fd, int output_fd) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main(void) {
    char *args[MAX_LINE / 2 + 1]; 
    int should_run = 1; 

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    while (should_run) {
        printf("it007sh>"); 
        fflush(stdout);

        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);
        input[strcspn(input, "\n")] = '\0'; 

        add_to_history(input);
	
	if (strcmp(input, "exit") == 0) {
            should_run = 0;  
            continue;
        }

        if (strcmp(input, "HF") == 0) {
            print_history();
            continue; 
        }
	
        char *token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; 

        int redirect_output = 0;
        int redirect_input = 0;
        int has_pipeline = 0;

        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], ">") == 0) {
                redirect_output = 1;
                args[j] = NULL; 
                int fd = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                execute_with_redirection(args, STDIN_FILENO, fd);
                break;
            } else if (strcmp(args[j], "<") == 0) {
                redirect_input = 1;
                args[j] = NULL;
                int fd = open(args[j + 1], O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                execute_with_redirection(args, fd, STDOUT_FILENO);
                break;
            } else if (strcmp(args[j], "|") == 0) {
                has_pipeline = 1;
                args[j] = NULL; 
                char *args2[MAX_LINE / 2 + 1];
                int k = 0;
                for (int l = j + 1; l < i; l++) {
                    args2[k++] = args[l];
                }
                args2[k] = NULL; 
                execute_pipeline(args, args2); 
                break;
            }
        }

        if (!redirect_output && !redirect_input && !has_pipeline) {
            execute_command(args);
        }
    }

    return 0;
}

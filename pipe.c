#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void handle_error(const char* msg);
int first_program(char* program);
void last_program(char* program, int read);
int regular_case(char* program, int read);

int main(int argc, char *argv[])
{
	if (argc <= 1) {
        perror("invalid args"); // msg to stderr
        errno = EINVAL; // error code for invalid arg
        exit(errno);
    }
    else if (argc == 2) { // 1 process: pipe is not needed
        execlp(argv[1], argv[1], NULL);
    }
    else { // > 1 process
        int read;
        read = first_program(argv[1]); // get read end of 1st pipe
        for (int i = 2; i < argc - 1; i++) // feed read end into program
            read = regular_case(argv[i], read);
        last_program(argv[argc - 1], read);
    }
}

void handle_error(const char* msg) {
    perror(msg);
    exit(errno);
}

int first_program(char* program) {
    int fildes[2];
    if (pipe(fildes) == -1)
        handle_error("piping error");
    
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            handle_error("process creation failure");
        case 0:
            dup2(fildes[1], STDOUT_FILENO); // stdout points to pipe's write end
            close(fildes[1]); // close the file descriptor
            execlp(program, program, NULL); // replace current process w/ new process
            handle_error("execlp: process unable to be created");
        default:
            // int status; // delete!!
            wait(&status); // wait until child to change state
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0)
                    exit(exit_status);
            }
            close(fildes[1]);
            return fildes[0]; // get the pipe's read end
    }
    return -1;
}

void last_program(char* program, int read) {
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            handle_error("process creation failure");
        case 0:
            dup2(read, STDIN_FILENO); // stdin points to previous pipe's read end
            close(read);
            execlp(program, program, NULL);
            handle_error("execlp: process unable to be created");
        default:
            // int status; // delete!!
            wait(&status); // wait until child to change state
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0)
                    exit(exit_status);
            }
            close(read); // right place?
    }
}

int regular_case(char* program, int read) {
    int fildes[2];
    if (pipe(fildes) == -1)
        handle_error("piping error");
    
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            handle_error("process creation failure");
        case 0:
            dup2(read, STDIN_FILENO);
            dup2(fildes[1], STDOUT_FILENO); 
            close(fildes[0]); // pipe's read end not needed for child process
            close(fildes[1]);
            execlp(program, program, NULL);
            handle_error("execlp: process unable to be created");
        default:
            // int status; // delete!!
            wait(&status); // wait until child to change state
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0)
                    exit(exit_status);
            }
            close(read); // right place?
            close(fildes[1]); // right place?
            return fildes[0];
    }
    return -1;
}

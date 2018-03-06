/*
Write a Unix program that can be run in command line: you_program_name file_name, 
where file_name is in the current directory

When the program starts, it does the following

    If filename is not specified in the command line, or if there are too many parameters, 
    display the correct usage and then exit. Otherwise,
    
    It forks three (3) child processes
    The parent process then displays its own PID information only once, then waits for its child processes die.
    Let one child-process run the "ls -l" command (using the "execl" system call);
    Let another child-process run the "ps -ef" command;
    Let the third child-process display the content of the file (specified by file_name). You can use the program "more" or "cat" to display it.
    After all child processes terminate, the main process displays "main process terminates" then exits.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

void fork_process(char* filename);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        perror("Too few arguments. Needs file name...\n"); exit(1);
    } else if (argc > 3) {
        perror("Too many arguments. Only one file name needed...\n"); exit(1);
    } else {
        fork_process(argv[1]);
    }
    return 0;
}

void fork_process(char* filename) {
    printf("Parent PID is %d\n", (int) getpid());
    pid_t pid = vfork();
    if (pid < 0) {
        perror("Error while forking\n"); exit(1);
    } else if (pid == 0) {
        printf("Listing current directory...\n");
        execl("/bin/ls", "ls", "-l", (char*) NULL); exit(0);
    }
    pid = vfork();
    if (pid < 0) {
        perror("Error while forking\n"); exit(1);
    } else if (pid == 0) {
        printf("Listing running processes...\n");
        execl("/bin/ps", "ps", "-ef", (char *) NULL); exit(0);
    }
    pid = vfork();
    if (pid < 0) {
        perror("Error while forking\n"); exit(1);
    } else if (pid == 0) {
        printf("Displaying content of a file...\n");
        execl("/bin/cat", "cat", filename, (char*) NULL); exit(0);
    }
    while ((pid = waitpid(-1, NULL, 0))) {
        if (errno == ECHILD) break;
    }
    printf("Main process terminates...\n");
}

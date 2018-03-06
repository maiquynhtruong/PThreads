/**
A Unix program that can be run in command line using the following syntax: program_name N, where N is a number

When the program starts, it does the following
    1) If N is not specified in the command line, or the parameter is not a number, 
        or if there are too many parameters, display the correct usage and then exit.
    2) It  forks three (3) child processes
        2a)  The main process repeatedly displays 
            "This is the main process, my PID is ....", pauses for about 2 seconds, 
            then displays the above information again, for a total of N times.
        2b)  Each of the three child processes repeatedly displays
             "This is a child process, my PID is ..., my parent PID is ...",
             then pauses for about 2 seconds, then displays the above information again, for a total of N times.
*/
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
void fork_processes(int N); 
int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("Not enough parameter: N number needed!"); exit(1);
    } else if (argc > 2) {
        perror("Too many parameters: Only one number allowed!"); exit(1);
    } else {
        fork_processes(atoi(argv[1]));
    }
    return 0;
}
void fork_processes(int N) {
    int i;
    for (i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error while forking\n"); exit(1);
        } else if (pid == 0) { // child process
            for (i = 0; i < N; i++) {
                printf("This is a child process, my PID is %d, my parent PID is %d\n", (int) getpid(), (int) getppid()); 
                sleep(2);
            }
            exit(0); // child won't escape its block
        } 
    }
    for (i = 0; i < N; i++) {
        printf("This is the main process, my PID is %d\n", (int) getpid());
        sleep(2);
    }
    exit(0);
}


#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#include "err.h"
#include "utils.h"


#define BUFFER_SIZE 1024


int main() {
    int pipe_dsc[2];

    char programName[BUFFER_SIZE] = "sleep";
    char args_str[BUFFER_SIZE] = "sleep 5";
    char buffer[BUFFER_SIZE];
    pid_t status = -1;

    memset(buffer, BUFFER_SIZE, 0);

    char **args = split_string(args_str);

    if (pipe(pipe_dsc) == -1) syserr("Error in pipe\n");

    pid_t child_pid = fork();

    switch (child_pid) {
    case -1:
        syserr("Error in fork\n");

    case 0:
        if (close (STDIN_FILENO) == -1)            syserr("Error in child, close (0)\n");
        if (close (pipe_dsc [0]) == -1) syserr("Error in child, close (pipe_dsc [0])\n");
        if (dup2(pipe_dsc[1],STDOUT_FILENO) == -1)            syserr("Error in dup\n");
        if (close (pipe_dsc [1]) == -1) syserr("Error in child, close (pipe_dsc [1])\n");

        execvp(programName, args);
        syserr("Error in execvp\n");

        exit (1);

    default:
        if (close (pipe_dsc [1]) == -1) syserr("Error in parent, close (pipe_dsc [0])\n");

//        if (read (pipe_dsc [0], buffer, BUFFER_SIZE - 1) == -1)
//            syserr("Error in read\n");

        if (close (pipe_dsc [0]) == -1) syserr("Error in parent, close (pipe_dsc [1])\n");

        printf("essa rodzic \n");
        kill(child_pid, SIGKILL);

//        int check = waitpid(0, &status, WNOHANG);

//        sleep(1);

        if ( waitpid(0, &status, 0) == -1)
            syserr("Error in wait\n");



        for(int i = 0; i < 20; i++) {
            printf("%c |", buffer[i]);
        }
        printf("\n");


        if (!WIFEXITED(status)) {
            printf("signalled.\n", status);
        } else {
            printf("status: %d.\n", status);
        }

        free_split_string(args);

        exit (0);
    }
}
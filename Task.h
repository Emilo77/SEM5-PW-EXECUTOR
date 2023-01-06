#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include <cstdint>
#include <cstring>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "err.h"

#define MAX_LINE_SIZE 1024
#define NOT_DONE -1

using std::string;

class Task {
private:
    char* programName;
    char** args;

    id_t id;
    pthread_t thread_id;
    pid_t exec_pid;

    char lastLineOut[MAX_LINE_SIZE];
    char lastLineErr[MAX_LINE_SIZE];

    int status{NOT_DONE};
    int signal{NOT_DONE};

    int pipe_fd_out[2];
    int pipe_fd_err[2];

public:
    Task(id_t id, char* programName, char** args)
        : id(id)
        , programName(programName)
        , args(args)
    {
        memset(lastLineOut, 0, MAX_LINE_SIZE);
        memset(lastLineErr, 0, MAX_LINE_SIZE);

        if (pipe(pipe_fd_out) == -1) syserr("Error in pipe\n");
        if (pipe(pipe_fd_err) == -1) syserr("Error in pipe\n");
    }

    void start_process() {
        exec_pid = fork();

        switch (exec_pid) {
        case -1:
            syserr("Error in fork\n");

        case 0:
            if (close (STDIN_FILENO) == -1) syserr("Error in child, close (0)\n");
            if (dup2 (pipe_fd_out[0], STDIN_FILENO) != 0) syserr("Error in child, dup (pipe_fd_out [0])\n");
            if (close (pipe_fd_out[0]) == -1) syserr("Error in child, close (pipe_fd_out [0])\n");
            if (close (pipe_fd_out[1]) == -1) syserr("Error in child, close (pipe_fd_out [1])\n");

            execvp(programName, args);

            syserr("Error in execvp\n");
            exit (1);

        default:
            if (close (pipe_fd_out[1]) == -1)
                syserr("Error in parent, close (pipe_fd_out [1])\n");

            if (read(pipe_fd_out[0], lastLineOut, MAX_LINE_SIZE - 1) == -1)
                syserr("Error in read\n");

            if (close (pipe_fd_out[0]) == -1)
                syserr("Error in parent, close (pipe_fd_out [0])\n");

            if (wait (0) == -1)
                syserr("Error in wait\n");
            exit (0);
        }
    }
};

#endif // EXECUTOR_TASK_H

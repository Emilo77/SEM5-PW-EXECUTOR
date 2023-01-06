#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include <cstdint>
#include <cstring>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

extern "C" {
    #include "err.h"
    #include "utils.h"
};



#define MAX_LINE_SIZE 1024
#define NOT_DONE (-1)

class Task {
private:
    char* programName;
    char** args;

    id_t taskId;

    pthread_attr_t attr;
    pthread_t signalThread;
    pthread_t outThread;
    pthread_t errThread;

    pid_t execPid;

    char lastLineOut[MAX_LINE_SIZE];
    char lastLineErr[MAX_LINE_SIZE];

    int status { NOT_DONE };
    int signal { NOT_DONE };

    int pipeFdOut[2];
    int pipeFdErr[2];

    char* getLastLineOut();

    char* getLastLineErr();

    void start_process();

    void create_signal_thread();

public:

    Task(id_t id, char* programName, char** args)
        : taskId(id)
        , programName(programName)
        , args(args)
    {
        memset(lastLineOut, 0, MAX_LINE_SIZE);
        memset(lastLineErr, 0, MAX_LINE_SIZE);

        if (pipe(pipeFdOut) == -1)
            syserr("Error in out pipe\n");
        if (pipe(pipeFdErr) == -1)
            syserr("Error in err pipe\n");
    }

    void print_started();
    void print_out();
    void print_err();
    void print_ended();

    void send_signal();
};

#endif // EXECUTOR_TASK_H

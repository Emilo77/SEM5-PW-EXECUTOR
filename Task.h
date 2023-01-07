#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include "Synchronizer.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>



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

    Synchronizer synchronizer;
    pthread_mutex_t lockLineOut;
    pthread_mutex_t lockLineErr;

    pid_t execPid;

    char lastLineOut[MAX_LINE_SIZE];
    char lastLineErr[MAX_LINE_SIZE];

    int status { NOT_DONE };
    bool signal { false };

    int pipeFdOut[2];
    int pipeFdErr[2];

    void openPipes();

    void initLocks();

    void destroyLocks();

    void startProcess();

    void createSignalThread();

public:
    Task(id_t id,
        char* programName,
        char** args,
        Synchronizer &sync)
        : taskId(id)
        , programName(programName)
        , args(args)
        , synchronizer(sync)
    {
        memset(lastLineOut, 0, MAX_LINE_SIZE);
        memset(lastLineErr, 0, MAX_LINE_SIZE);

        openPipes();

    }

    void sendSignal(int sig) {
//        kill(0, sig); //todo zmienić 0 na pid dziecka
    }

    void waitForProgramEnd();

    void printStarted();
    void printOut();
    void printErr();
    void printEnded();

    void closeTask();

    void execute();
};

#endif // EXECUTOR_TASK_H

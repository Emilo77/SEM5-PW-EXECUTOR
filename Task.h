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
    struct TaskParams {
        id_t taskId;

        char* programName;
        char** args;

        int pipeFdOut[2];
        int pipeFdErr[2];

        char lastLineOut[MAX_LINE_SIZE];
        char lastLineErr[MAX_LINE_SIZE];

        pthread_mutex_t lockLineOut;
        pthread_mutex_t lockLineErr;

        pthread_attr_t attr;
        pthread_t outThread;
        pthread_t errThread;

        pid_t execPid;

        bool signal { false };
        int status { NOT_DONE };
    };
private:
    TaskParams taskParams;
    pthread_t mainHelperThread{};

    Synchronizer synchronizer;

    void openPipes();

    void initLocks();

    void initThreadAttr();

    void destroyLocks();

    void startMainHelper();
    void startReaders();

    static void* startExecProcess(TaskParams* threadArgs);
    static void* waitForExecEnd(TaskParams* threadArgs);
    static void* printEnded(TaskParams* threadArgs);
    static void* mainHelper(void *arg);
    static void* outReader(void* arg);
    static void* errReader(void* arg);

public:
    Task(id_t id,
        char* programName,
        char** args,
        Synchronizer &sync)
        : synchronizer(sync)
    {
        memset(taskParams.lastLineOut, 0, MAX_LINE_SIZE);
        memset(taskParams.lastLineErr, 0, MAX_LINE_SIZE);

        taskParams.taskId = id;
        taskParams.programName = programName;
        taskParams.args = args;
    }

    void sendSignal(int sig) {
//        kill(0, sig); //todo zmienić 0 na pid dziecka
    }

    void printStarted();
    void printOut();
    void printErr();

    void startTask();
    void closeTask();
};

#endif // EXECUTOR_TASK_H

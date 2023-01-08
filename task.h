#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "synchronizer.h"

#define MAX_LINE_SIZE 1024
#define MAX_TASKS 512 // todo zmienić
#define NOT_DONE (-1)


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

    bool signal;
    int status;
};

struct Task {
    struct TaskParams taskParams;
    pthread_t mainHelperThread;
};

extern struct Task taskArray[MAX_TASKS];

void initLocks(struct TaskParams* p);

void initThreadAttr(pthread_attr_t *attr);

void destroyLocks(struct TaskParams* p);

static void* startExecProcess(struct TaskParams* p);
static void* waitForExecEnd(struct TaskParams* p);
static void* printEnded(struct TaskParams* params);
static void* mainHelper(void* arg);
static void* outReader(void* arg);
static void* errReader(void* arg);

struct Task newTask(id_t id, char* programName, char** args);

void sendSignal(struct Task* t, int sig);

void printStarted(struct TaskParams* params);
void printOut(struct TaskParams* params);
void printErr(struct TaskParams* params);

void startTask(struct Task* t);
void closeTask(struct Task* t);

#endif // EXECUTOR_TASK_H

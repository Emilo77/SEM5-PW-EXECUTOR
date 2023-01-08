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

struct Task {
    char* programName;
    char** args;

    pthread_t mainHelperThread;
    pthread_mutex_t lockLineOut;
    pthread_mutex_t lockLineErr;
    pthread_mutex_t lockPidWaiting;

    id_t taskId;
    pid_t execPid;
    bool signal;
    int status;

    int pipeFdOut[2];
    int pipeFdErr[2];

    char lastLineOut[MAX_LINE_SIZE];
    char lastLineErr[MAX_LINE_SIZE];

    pthread_t outThread;
    pthread_t errThread;

};

extern struct Task taskArray[MAX_TASKS];

void initLocks(id_t taskId);
void destroyLocks(id_t taskId);

static void* startExecProcess(struct Task* task);
static void* waitForExecEnd(struct Task* task);
static void* printEnded(struct Task* task);
static void* mainHelper(void* arg);
static void* outReader(void* arg);
static void* errReader(void* arg);

struct Task *newTask(id_t id, char* programName, char** args);

void sendSignal(id_t taskId, int sig);

void printStarted(id_t taskId);
void executeOut(id_t taskId);
void executeErr(id_t taskId);

void startTask(id_t taskId);
void closeTask(id_t taskId);

#endif // EXECUTOR_TASK_H

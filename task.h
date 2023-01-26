#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#include "synchronizer.h"

#define MAX_LINE_SIZE 1024
#define MAX_TASKS 4096
#define NOT_DONE (-1)

/* Struktura przechowująca informacje na temat zadania */
struct Task {
    /* Argumenty programu */
    char* programName;
    char** args;

    /* Wątki pomocnicze */
    pthread_t mainHelperThread;
    pthread_t outThread;
    pthread_t errThread;

    /* Semafory */
    sem_t lockLineOut;
    sem_t lockLineErr;
    sem_t lockPidWaiting;

    /* Synchronizacja z wątkiem głównym */
    struct Synchronizer* sync;

    id_t taskId;
    pid_t execPid;
    bool signal;
    int status;

    /* Deskryptory do komunikacji z procesem
     * wykonującym program */
    int pipeFdOut[2];
    int pipeFdErr[2];

    /* Bufory na ostatnie linie */
    char lastLineOut[MAX_LINE_SIZE];
    char lastLineErr[MAX_LINE_SIZE];
};

extern struct Task taskArray[MAX_TASKS];

/* Operacje na semaforach */
void initSemaphores(id_t taskId);
void destroySemaphores(id_t taskId);

/* Funkcje pomocnicze, obsługiwane przez pomocniczy wątek główny */
void* startExecProcess(struct Task* task);
void* waitForExecEnd(struct Task* task);
void* closePipesOnExec(struct Task* task);
void* printEnded(struct Task* task);

/* Funkcja obsługiwana przez główny wątek pomocniczy */
static void* mainHelper(void* arg);

/* Funkcja obsługiwana przez wątek zapisujący
 * linię standardowego wyjścia do bufora */
static void* outReader(void* arg);

/* Funkcja obsługiwana przez wątek zapisujący linię
 * wyjścia błędów do bufora */
static void* errReader(void* arg);

/* Funkcja inicjująca nowe zadanie */
struct Task *newTask(id_t id, char* programName, char** args,
    struct Synchronizer *sync);

/* Wysłanie sygnału do procesu z danego zadania */
void sendSignal(id_t taskId, int sig);

/* Funkcje wypisujące na standardowe wyjście executora */
void printStarted(id_t taskId);
void executeOut(id_t taskId);
void executeErr(id_t taskId);

/* Rozpoczęcie obsługi zadania */
void startTask(id_t taskId);

/* Zakończenie obsługi zadania
 * pod koniec działania programu */
void closeTask(id_t taskId);

#endif // EXECUTOR_TASK_H

#ifndef MIMUW_FORK_SYNCHRONIZER_H
#define MIMUW_FORK_SYNCHRONIZER_H

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#include "err.h"
#include "utils.h"

struct Synchronizer {
    sem_t* mutex;
    sem_t* executor;
    sem_t* printer;
     int waitingToPrint;
     int printing;
     int waitingToExec;
     int executing;
};

void synchronizerInit(struct Synchronizer* s);

void preProtocolExecutor(struct Synchronizer* s);
void postProtocolExecutor(struct Synchronizer* s);
void preProtocolPrinter(struct Synchronizer* s);
void postProtocolPrinter(struct Synchronizer* s);

void tryToLock(sem_t* m);
void tryToUnlock(sem_t* m);

void synchronizerDestroy(struct Synchronizer* s);

#endif // MIMUW_FORK_SYNCHRONIZER_H

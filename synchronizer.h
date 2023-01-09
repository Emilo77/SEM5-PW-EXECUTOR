#ifndef MIMUW_FORK_SYNCHRONIZER_H
#define MIMUW_FORK_SYNCHRONIZER_H

#include <pthread.h>
#include <stdlib.h>

#include "err.h"
#include "utils.h"

struct Synchronizer {
    pthread_mutex_t* mutex;
    pthread_mutex_t* executor;
    pthread_mutex_t* printing;

    int waitingToPrint;
};

void synchronizerInit(struct Synchronizer* s);

void preProtocolExecutor(struct Synchronizer* s);
void postProtocolExecutor(struct Synchronizer* s);
void preProtocolPrinter(struct Synchronizer* s);
void postProtocolPrinter(struct Synchronizer* s);

void tryToLock(pthread_mutex_t* m);
void tryToUnlock(pthread_mutex_t* m);

void synchronizerDestroy(struct Synchronizer* s);

#endif // MIMUW_FORK_SYNCHRONIZER_H

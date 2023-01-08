#ifndef MIMUW_FORK_SYNCHRONIZER_H
#define MIMUW_FORK_SYNCHRONIZER_H

#include <pthread.h>
#include <stdlib.h>

#include "err.h"
#include "utils.h"

extern struct Synchronizer synchronizer;

struct Synchronizer {
    pthread_mutex_t* mutex;
    pthread_mutex_t* executor;
    pthread_mutex_t* printing;

    int waitingToPrint;
};

void synchronizerInit(struct Synchronizer* s);

void preProtocolExecutor(struct Synchronizer* s);
void preProtocolPrinter(struct Synchronizer* s);
void postProtocolPrinter(struct Synchronizer* s);

static void tryToLock(pthread_mutex_t* m);
static void tryToUnlock(pthread_mutex_t* m);

void synchronizerDestroy(struct Synchronizer* s);

#endif // MIMUW_FORK_SYNCHRONIZER_H

#ifndef MIMUW_FORK_SYNCHRONIZER_H
#define MIMUW_FORK_SYNCHRONIZER_H

#include <pthread.h>

extern "C" {
#include "err.h"
#include "utils.h"
};

class Synchronizer {
private:
    pthread_mutex_t mutex;
    pthread_mutex_t executor;
    pthread_mutex_t printing;

    int waitingToPrint{0};

    void init();

    static void tryToLock(pthread_mutex_t& m);
    static void tryToUnlock(pthread_mutex_t& m);

public:
    Synchronizer() { init(); }

    void preProtocolExecutor();

    void preProtocolPrinter();
    void postProtocolPrinter();

    void destroy();
};

#endif // MIMUW_FORK_SYNCHRONIZER_H

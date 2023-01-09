#include "synchronizer.h"

struct Synchronizer synchronizer;

void synchronizerInit(struct Synchronizer* s)
{
    s->mutex = (pthread_mutex_t*) malloc(sizeof (pthread_mutex_t));
    s->executor = (pthread_mutex_t*) malloc(sizeof (pthread_mutex_t));
    s->printing = (pthread_mutex_t*) malloc(sizeof (pthread_mutex_t));

    // todo ustawić na początku na 1
    if (pthread_mutex_init(s->mutex, 0) != 0)
        syserr("synchronizerInit lock mutex failed");
    // todo ustawić na początku na 0
    if (pthread_mutex_init(s->executor, 0) != 0)
        syserr("synchronizerInit lock initExecutor failed");
    // todo ustawić na początku na 0
    if (pthread_mutex_init(s->printing, 0) != 0)
        syserr("synchronizerInit lock printing failed");
}

void synchronizerDestroy(struct Synchronizer* s)
{
    if (pthread_mutex_destroy(s->mutex) != 0)
        syserr("synchronizerDestroy lock mutex failed");
    if (pthread_mutex_destroy(s->executor) != 0)
        syserr("synchronizerDestroy lock initExecutor failed");
    if (pthread_mutex_destroy(s->printing) != 0)
        syserr("synchronizerDestroy lock printing failed");

    free(s->mutex);
    free(s->executor);
    free(s->printing);
}

void tryToLock(pthread_mutex_t* m)
{
    if (pthread_mutex_lock(m) != 0)
        syserr("lock failed");
}

void tryToUnlock(pthread_mutex_t* m)
{
    if (pthread_mutex_unlock(m) != 0)
        syserr("unlock failed");
}

void preProtocolExecutor(struct Synchronizer* s)
{
    tryToLock(s->mutex);
}

void postProtocolExecutor(struct Synchronizer* s)
{
    tryToUnlock(s->mutex);
}

void preProtocolPrinter(struct Synchronizer* s)
{
    tryToLock(s->printing);
}

void postProtocolPrinter(struct Synchronizer* s)
{
    tryToUnlock(s->executor);
}

#include "synchronizer.h"

void synchronizerInit(struct Synchronizer* s)
{
    s->mutex = (sem_t*) malloc(sizeof (sem_t));
    s->executor = (sem_t*) malloc(sizeof (sem_t));
    s->printing = (sem_t*) malloc(sizeof (sem_t));

    /* semafor ustawiony na 1 */
    if (sem_init(s->mutex, 0, 1) == -1)
        syserr("semaphore mutex init failed");
    /* semafor ustawiony na 0 */
    if (sem_init(s->executor, 0, 0) == -1)
        syserr("semaphore executor init failed");
    /* semafor ustawiony na 0 */
    if (sem_init(s->printing, 0, 0) == -1)
        syserr("semaphore printing init failed");
}

void synchronizerDestroy(struct Synchronizer* s)
{
    if (sem_destroy(s->mutex) == -1)
        syserr("semaphore mutex destroy failed");
    if (sem_destroy(s->executor) == -1)
        syserr("semaphore executor destroy failed");
    if (sem_destroy(s->printing) == -1)
        syserr("semaphore printing destroy failed");

    free(s->mutex);
    free(s->executor);
    free(s->printing);
}

void tryToLock(sem_t* m)
{
    if (sem_wait(m) == -1)
        syserr("lock failed");
}

void tryToUnlock(sem_t* m)
{
    if (sem_post(m) == -1)
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
    tryToLock(s->mutex);
}

void postProtocolPrinter(struct Synchronizer* s)
{
    tryToUnlock(s->mutex);
}

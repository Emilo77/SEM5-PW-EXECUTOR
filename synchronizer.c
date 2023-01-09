#include "synchronizer.h"

void synchronizerInit(struct Synchronizer* s)
{
    s->waitingToPrint = 0;
    s->waitingToExec = 0;
    s->printing = 0;
    s->executing = 0;

    /* Alokacja pamięci na semafory */
    s->mutex = (sem_t*)malloc(sizeof(sem_t));
    s->executor = (sem_t*)malloc(sizeof(sem_t));
    s->printer = (sem_t*)malloc(sizeof(sem_t));

    /* Semafor ustawiony na 1 */
    if (sem_init(s->mutex, 0, 1) == -1)
        syserr("semaphore mutex init failed");
    /* Semafor ustawiony na 0 */
    if (sem_init(s->executor, 0, 0) == -1)
        syserr("semaphore executor init failed");
    /* Semafor ustawiony na 0 */
    if (sem_init(s->printer, 0, 0) == -1)
        syserr("semaphore printer init failed");
}

void synchronizerDestroy(struct Synchronizer* s)
{
    if (sem_destroy(s->mutex) == -1)
        syserr("semaphore mutex destroy failed");
    if (sem_destroy(s->executor) == -1)
        syserr("semaphore executor destroy failed");
    if (sem_destroy(s->printer) == -1)
        syserr("semaphore printer destroy failed");

    free(s->mutex);
    free(s->executor);
    free(s->printer);
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

    if (s->printing + s->waitingToPrint > 0) {
        s->waitingToExec++;

        tryToUnlock(s->mutex);
        tryToLock(s->executor);

        s->waitingToExec--;
    }
    s->executing++;
    tryToUnlock(s->mutex);
}

void postProtocolExecutor(struct Synchronizer* s)
{
    tryToLock(s->mutex);
    s->executing--;

    if (s->waitingToPrint > 0) {
        tryToUnlock(s->printer);

    } else {
        tryToUnlock(s->mutex);
    }
}

void preProtocolPrinter(struct Synchronizer* s)
{
    tryToLock(s->mutex);

    if (s->executing || s->printing) {
        s->waitingToPrint++;

        tryToUnlock(s->mutex);
        tryToLock(s->printer);

        s->waitingToPrint--;
    }
    s->printing++;
    tryToUnlock(s->mutex);
}

void postProtocolPrinter(struct Synchronizer* s)
{
    tryToLock(s->mutex);
    s->printing--;

    if (s->waitingToPrint > 0) {
        tryToUnlock(s->printer);

    } else if (s->waitingToExec > 0) {
        tryToUnlock(s->executor);

    } else {
        tryToUnlock(s->mutex);
    }
}

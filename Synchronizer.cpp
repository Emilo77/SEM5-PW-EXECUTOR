#include "Synchronizer.h"

void Synchronizer::init()
{
    //todo ustawić na początku na 1
    if (pthread_mutex_init(&mutex, 0) != 0)
        syserr ("init lock mutex failed");
    //todo ustawić na początku na 0
    if (pthread_mutex_init(&executor, 0) != 0)
        syserr ("init lock executor failed");
    //todo ustawić na początku na 0
    if (pthread_mutex_init(&printing, 0) != 0)
        syserr ("init lock printing failed");
}

void Synchronizer::destroy()
{
    if (pthread_mutex_destroy(&mutex) != 0)
        syserr ("destroy lock mutex failed");
    if (pthread_mutex_destroy(&executor) != 0)
        syserr ("destroy lock executor failed");
    if (pthread_mutex_destroy(&printing) != 0)
        syserr ("destroy lock printing failed");
}

void Synchronizer::tryToLock(pthread_mutex_t &m)
{
    if (pthread_mutex_lock(&m) != 0)
        syserr ("lock failed");
}
void Synchronizer::tryToUnlock(pthread_mutex_t &m)
{
    if (pthread_mutex_unlock(&m) != 0)
        syserr ("unlock failed");

}
void Synchronizer::preProtocolExecutor()
{
    tryToLock(mutex);
    if (waitingToPrint != 0) {
        tryToUnlock(printing);
        tryToLock(executor);
    } else {
        tryToUnlock(mutex);
    }

}

void Synchronizer::preProtocolPrinter()
{
    tryToLock(mutex);
    waitingToPrint++;
    tryToUnlock(mutex);

    tryToLock(printing);
}
void Synchronizer::postProtocolPrinter()
{
    waitingToPrint--;
    if (waitingToPrint != 0) {
        tryToUnlock(printing);
    } else {
        tryToUnlock(mutex);
        tryToUnlock(executor);
    }

}

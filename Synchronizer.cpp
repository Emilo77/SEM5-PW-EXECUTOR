#include "Synchronizer.h"

void Synchronizer::init()
{
    if (pthread_mutex_init(&mutex, 0) != 0)
        syserr ("init lock mutex failed");
    if (pthread_mutex_init(&executor, 0) != 0)
        syserr ("init lock executor failed");
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
    //todo z kartki
}
void Synchronizer::postProtocolExecutor()
{
    //todo z kartki
}
void Synchronizer::preProtocolPrinter()
{
    //todo z kartki
}
void Synchronizer::postProtocolPrinter()
{
    //todo z kartki
}

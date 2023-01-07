#include "Task.h"

void Task::openPipes()
{
    if (pipe(pipeFdOut) == -1)
        syserr("Error in out pipe\n");
    if (pipe(pipeFdErr) == -1)
        syserr("Error in err pipe\n");
}

void Task::initLocks() //todo może do zmiany
{
    if (pthread_mutex_init(&lockLineOut, 0) != 0)
        syserr ("init lockLineOut failed");
    if (pthread_mutex_init(&lockLineErr, 0) != 0)
        syserr ("init lockLineErr failed");

}

void Task::destroyLocks()
{
    if (pthread_mutex_destroy(&lockLineOut) != 0)
        syserr ("destroy lockLineOut failed");
    if (pthread_mutex_destroy(&lockLineErr) != 0)
        syserr ("destroy lockLineErr failed");
}

void Task::printStarted()
{
    printf("Task %d started: pid %d.\n", taskId, execPid);
}

void Task::printOut()
{
    // podnieś mutex na ochronę
    printf("Task %d stdout: '%s'.\n", taskId, lastLineOut);
    // opuść mutex na ochronę
}

void Task::printErr()
{
    // podnieś mutex na ochronę
    printf("Task %d stderr: '%s'.\n", taskId, lastLineErr);
    // opuść mutex na ochronę
}

void Task::printEnded()
{
    if ((status == NOT_DONE) && !signal) {
        syserr("Task is not done yet.");
    }

    if (signal) {
        printf("Task %d ended: signalled.\n", taskId);

    } else {
        printf("Task %d ended: status %d.\n", taskId, status);
    }
}

void Task::startProcess()
{
    execPid = fork();

    switch (execPid) {
    /* Niepowodzenie funkcji fork */
    case -1:
        syserr("Error in fork\n");

    /* Proces-dziecko utworzone przez fork */
    case 0:
        //        /* Zamknięcie deskryptora na standardowe wejście */
        //        if (close(STDIN_FILENO) == -1)
        //            syserr("Error in child, close (0)\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za czytanie */
        if (close(pipeFdOut[0]) == -1)
            syserr("Error in child, close (pipeFdOut [0])\n");
        if (close(pipeFdErr[0]) == -1)
            syserr("Error in child, close (pipeFdErr [0])\n");

        /* Zamiana STDOUT i STDERR na odpowiednie deskryptory łączy */
        if (dup2(pipeFdOut[1], STDOUT_FILENO) != 0)
            syserr("Error in child, dup (pipeFdOut [1])\n");
        if (dup2(pipeFdErr[1], STDERR_FILENO) != 0)
            syserr("Error in child, dup (pipeFdErr [1])\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(pipeFdOut[1]) == -1)
            syserr("Error in child, close (pipeFdOut [1])\n");
        if (close(pipeFdErr[1]) == -1)
            syserr("Error in child, close (pipeFdErr [1])\n");

        /* Uruchomienie programu z podanymi argumentami */
        execvp(programName, args);

        /* Sytuacja, w której funkcja execvp nie powiodła się */
        syserr("Error in execvp\n");
        exit(1);

    /* Proces macierzysty */
    default:
        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(pipeFdOut[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");
        if (close(pipeFdErr[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");

        if (read(pipeFdOut[0], lastLineOut, MAX_LINE_SIZE - 1) == -1)
            syserr("Error in read\n");

        if (close(pipeFdOut[0]) == -1)
            syserr("Error in parent, close (pipeFdOut [0])\n");

        if (wait(0) == -1)
            syserr("Error in wait\n");
        exit(0);
    }
}
void Task::createSignalThread()
{

}

void Task::execute()
{

}
void Task::waitForProgramEnd()

{
    if ( waitpid(0, &status, 0) == -1)
        syserr("Error in wait\n");

    if (!WIFEXITED(status)) {
        signal = true;
    }
}

void Task::closeTask()
{
    //destroy mutexów

    sendSignal(SIGQUIT);
    waitForProgramEnd();

    // przerwanie wątków

}


#include "Task.h"

void Task::openPipes()
{
}

void Task::initLocks()
{
    if (pthread_mutex_init(&(taskParams.lockLineOut), 0) != 0)
        syserr("init lockLineOut failed");
    if (pthread_mutex_init(&(taskParams.lockLineErr), 0) != 0)
        syserr("init lockLineErr failed");
}

void Task::destroyLocks()
{
    if (pthread_mutex_destroy(&(taskParams.lockLineOut)) != 0)
        syserr("destroy lockLineOut failed");
    if (pthread_mutex_destroy(&(taskParams.lockLineErr)) != 0)
        syserr("destroy lockLineErr failed");
}

void Task::printStarted()
{
    printf("Task %d started: pid %d.\n", taskParams.taskId, taskParams.execPid);
}

void Task::printOut()
{
    // podnieś mutex na ochronę
    printf("Task %d stdout: '%s'.\n", taskParams.taskId, taskParams.lastLineOut);
    // opuść mutex na ochronę
}

void Task::printErr()
{
    // podnieś mutex na ochronę
    printf("Task %d stderr: '%s'.\n", taskParams.taskId, taskParams.lastLineErr);
    // opuść mutex na ochronę
}

void* Task::printEnded(TaskParams* threadArgs)
{
    if ((threadArgs->status == NOT_DONE) && !threadArgs->signal) {
        syserr("Task is not done yet.");
    }

    if (threadArgs->signal) {
        printf("Task %d ended: signalled.\n", threadArgs->taskId);

    } else {
        printf("Task %d ended: status %d.\n", threadArgs->taskId, threadArgs->  status);
    }
}

void* Task::startExecProcess(TaskParams* threadArgs)
{
    threadArgs->execPid = fork();

    switch (threadArgs->execPid) {
    /* Niepowodzenie funkcji fork */
    case -1:
        syserr("Error in fork\n");

    /* Proces-dziecko utworzone przez fork */
    case 0:
        /* Zamknięcie deskryptora na standardowe wejście */
        if (close(STDIN_FILENO) == -1)
            syserr("Error in child, close (0)\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za czytanie */
        if (close(threadArgs->pipeFdOut[0]) == -1)
            syserr("Error in child, close (pipeFdOut [0])\n");
        if (close(threadArgs->pipeFdErr[0]) == -1)
            syserr("Error in child, close (pipeFdErr [0])\n");

        /* Zamiana STDOUT i STDERR na odpowiednie deskryptory łączy */
        if (dup2(threadArgs->pipeFdOut[1], STDOUT_FILENO) == -1)
            syserr("Error in child, dup (pipeFdOut [1])\n");
        if (dup2(threadArgs->pipeFdErr[1], STDERR_FILENO) == -1)
            syserr("Error in child, dup (pipeFdErr [1])\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(threadArgs->pipeFdOut[1]) == -1)
            syserr("Error in child, close (pipeFdOut[1])\n");
        if (close(threadArgs->pipeFdErr[1]) == -1)
            syserr("Error in child, close (pipeFdErr[1])\n");

//        printf("%s\n", threadArgs->programName);
//        for(int i = 0; i < 5; i++) {
//            printf("%d ", threadArgs->programName[i]);
//        }
//        printf("\n");
//
//        printf("%d", )


        /* Uruchomienie programu z podanymi argumentami */
        execvp(threadArgs->programName, threadArgs->args);

        /* Sytuacja, w której funkcja execvp nie powiodła się */
        syserr("Error in execvp\n");
        exit(1);

    /* Proces macierzysty */
    default:
        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(threadArgs->pipeFdOut[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");
        if (close(threadArgs->pipeFdErr[1]) == -1)
            syserr("Error in parent, close (pipeFdErr [1])\n");

        return nullptr;
    }
}

void* Task::waitForExecEnd(TaskParams*threadArgs)

{
    if (waitpid(0, &(threadArgs->status), 0) == -1)
        syserr("Error in wait\n");

    if (!WIFEXITED(threadArgs->status)) {
        threadArgs->signal = true;
    }
}

void Task::initThreadAttr()
{
    if ((pthread_attr_init(&(taskParams.attr))) != 0)
        syserr("attr_init");

    if ((pthread_attr_setdetachstate(&(taskParams.attr),PTHREAD_CREATE_DETACHED)) != 0)
        syserr("set thread detachable");
}

void* Task::mainHelper(void* arg)
{
    auto* threadArgs = (TaskParams*) arg;

    /* Utworzenie łączy */
    if (pipe(threadArgs->pipeFdOut) == -1)
        syserr("Error in out pipe\n");
    if (pipe(threadArgs->pipeFdErr) == -1)
        syserr("Error in err pipe\n");

    startExecProcess(threadArgs);

    /* Stworzenie wątków pomocniczych */
//    if ((pthread_create(&(threadArgs->outThread), &(threadArgs->attr), outReader, &threadArgs)) != 0)
//        syserr("create outReader thread");
//    if ((pthread_create(&(threadArgs->errThread), &(threadArgs->attr), errReader, &threadArgs)) != 0)
//        syserr("create errReader thread");

    waitForExecEnd(threadArgs);

    printEnded(threadArgs);

//    if ((pthread_join(threadArgs->outThread, NULL)) != 0)
//        syserr("join of thread failed");
//    if ((pthread_join(threadArgs->outThread, NULL)) != 0)
//        syserr("join of thread failed");

    return nullptr;
}

void* Task::outReader(void* arg)
{
    auto* threadArgs = (TaskParams*)arg;

    FILE* f = fdopen(threadArgs->pipeFdOut[0], "r");

    // todo mutexy
    while (read_line(threadArgs->lastLineOut, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

//    if (close (threadArgs->pipeFdOut[0]) == -1)
//        syserr("Error in outReader, close (pipeFdOut[0])\n");

    return nullptr;
}

void* Task::errReader(void* arg)
{
    auto* threadArgs = (TaskParams*)arg;

    FILE* f = fdopen(threadArgs->pipeFdErr[0], "r");

    // todo mutexy
    while (read_line(threadArgs->lastLineErr, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

//    if (close (threadArgs->pipeFdErr[0]) == -1)
//        syserr("Error in errReader, close (pipeFdErr[0])\n");

    return nullptr;
}

void Task::startTask()
{
    initLocks();
    initThreadAttr();

    if ((pthread_create(&(mainHelperThread), &(taskParams.attr), mainHelper, &taskParams)) != 0)
        syserr("create MainHelper thread");

}

void Task::closeTask()
{
    sendSignal(SIGKILL); // todo może zamienić na SIGQUIT
    waitForExecEnd(&taskParams);

    destroyLocks();
}

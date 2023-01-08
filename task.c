#include "task.h"

struct Task newTask(id_t id, char* programName, char** args) {

    struct TaskParams newParams;

    newParams.taskId = id;
    newParams.programName = programName;
    newParams.args = args;

    memset(newParams.lastLineOut, 0, MAX_LINE_SIZE);
    memset(newParams.lastLineErr, 0, MAX_LINE_SIZE);

    struct Task newTask;
    newTask.taskParams = newParams;

    return newTask;
}

void initLocks(struct TaskParams* taskParams)
{
    if (pthread_mutex_init(&taskParams->lockLineOut, 0) != 0)
        syserr("synchronizerInit lockLineOut failed");
    if (pthread_mutex_init(&taskParams->lockLineErr, 0) != 0)
        syserr("synchronizerInit lockLineErr failed");
}

void destroyLocks(struct TaskParams* taskParams)
{
    if (pthread_mutex_destroy(&taskParams->lockLineOut) != 0)
        syserr("synchronizerDestroy lockLineOut failed");
    if (pthread_mutex_destroy(&taskParams->lockLineErr) != 0)
        syserr("synchronizerDestroy lockLineErr failed");
}

void sendSignal(struct Task* t, int sig)
{
    kill(t->taskParams.execPid, sig); //todo zmienić 0 na pid dziecka
}

void printStarted(struct TaskParams* taskParams)
{
    printf("Task %d started: pid %d.\n", taskParams->taskId, taskParams->execPid);
}

void printOut(struct TaskParams* taskParams)
{
    // podnieś mutex na ochronę
    printf("Task %d stdout: '%s'.\n", taskParams->taskId, taskParams->lastLineOut);
    // opuść mutex na ochronę
}

void printErr(struct TaskParams* taskParams)
{
    // podnieś mutex na ochronę
    printf("Task %d stderr: '%s'.\n", taskParams->taskId, taskParams->lastLineErr);
    // opuść mutex na ochronę
}

void* printEnded(struct TaskParams* threadArgs)
{
    if ((threadArgs->status == NOT_DONE) && !threadArgs->signal) {
        syserr("Task is not done yet.");
    }

    if (threadArgs->signal) {
        printf("Task %d ended: signalled.\n", threadArgs->taskId);

    } else {
        printf("Task %d ended: status %d.\n", threadArgs->taskId, threadArgs->status);
    }

    return NULL;
}

static void* startExecProcess(struct TaskParams* threadArgs)
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

        return NULL;
    }
}

void* waitForExecEnd(struct TaskParams* threadArgs)

{
    if (waitpid(0, &(threadArgs->status), 0) == -1)
        syserr("Error in wait\n");

    if (!WIFEXITED(threadArgs->status)) {
        threadArgs->signal = true;
    }
    return NULL;
}

void initThreadAttr(struct TaskParams* taskParams)
{
    if ((pthread_attr_init(&(taskParams->attr))) != 0)
        syserr("attr_init");

    if ((pthread_attr_setdetachstate(&(taskParams->attr), PTHREAD_CREATE_DETACHED)) != 0)
        syserr("set thread detachable");
}

void* mainHelper(void* arg)
{
    struct TaskParams* threadArgs = (struct TaskParams*)arg;

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

    return NULL;
}

void* outReader(void* arg)
{
    struct TaskParams* threadArgs = (struct TaskParams*)arg;

    FILE* f = fdopen(threadArgs->pipeFdOut[0], "r");

    // todo mutexy
    while (read_line(threadArgs->lastLineOut, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

    //    if (close (threadArgs->pipeFdOut[0]) == -1)
    //        syserr("Error in outReader, close (pipeFdOut[0])\n");

    return NULL;
}

void* errReader(void* arg)
{
    struct TaskParams* threadArgs = (struct TaskParams*)arg;

    FILE* f = fdopen(threadArgs->pipeFdErr[0], "r");

    // todo mutexy
    while (read_line(threadArgs->lastLineErr, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

    //    if (close (threadArgs->pipeFdErr[0]) == -1)
    //        syserr("Error in errReader, close (pipeFdErr[0])\n");

    return NULL;
}

void startTask(struct Task* t)
{
    initLocks(&t->taskParams);
    initThreadAttr(&t->taskParams);

    if ((pthread_create(&(t->mainHelperThread),
            &(t->taskParams.attr), mainHelper, &t->taskParams)) != 0)
        syserr("create MainHelper thread");
}

void closeTask(struct Task* t)
{
    sendSignal(t, SIGKILL); // todo może zamienić na SIGQUIT
    waitForExecEnd(&t->taskParams);

    destroyLocks(&t->taskParams);
}

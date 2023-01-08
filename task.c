#include "task.h"

struct Task taskArray[MAX_TASKS];

struct Task newTask(id_t id, char* programName, char** args)
{

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
    kill(t->taskParams.execPid, sig); // todo zmienić 0 na pid dziecka
}

void printStarted(struct TaskParams* params)
{
    printf("Task %d started: pid %d.\n", params->taskId, params->execPid);
}

void printOut(struct TaskParams* params)
{
    // podnieś mutex na ochronę
    printf("Task %d stdout: '%s'.\n", params->taskId, params->lastLineOut);
    // opuść mutex na ochronę
}

void printErr(struct TaskParams* params)
{
    // podnieś mutex na ochronę
    printf("Task %d stderr: '%s'.\n", params->taskId, params->lastLineErr);
    // opuść mutex na ochronę
}

void* printEnded(struct TaskParams* params)
{
    if ((params->status == NOT_DONE) && !params->signal) {
        syserr("Task is not done yet.");
    }

    if (params->signal) {
        printf("Task %d ended: signalled.\n", params->taskId);

    } else {
        printf("Task %d ended: status %d.\n", params->taskId, params->status);
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

//        sleep(1);
//
//        FILE* f = fdopen(threadArgs->pipeFdOut[0], "r");
//
//        // todo mutexy
//        while (read_line(threadArgs->lastLineOut, MAX_LINE_SIZE - 1, f)) {}
//
//        printf("LastLine: %s\n", threadArgs->lastLineOut);
//        fclose(f);

        return NULL;
    }
}

void* waitForExecEnd(struct TaskParams* threadArgs)

{
    //    pid_t pid = waitpid(0, &(threadArgs->status), 0);
    //    if (pid == -1 && errno != SIGCHLD) {
    //        syserr("Error in wait\n");
    //    }

    if (!WIFEXITED(threadArgs->status)) {
        threadArgs->signal = true;
    }
    return NULL;
}

void initThreadAttr(pthread_attr_t *attr)
{
    if ((pthread_attr_init(attr)) != 0)
        syserr("attr_init");

    if ((pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) != 0)
        syserr("set thread detachable");
}

void* mainHelper(void* arg)
{

    id_t taskId = *((id_t*)arg);
    free(arg);

    struct TaskParams* params = &taskArray[taskId].taskParams;

    /* Utworzenie łączy */
    if (pipe(params->pipeFdOut) == -1)
        syserr("Error in out pipe\n");
    if (pipe(params->pipeFdErr) == -1)
        syserr("Error in err pipe\n");

    startExecProcess(params);

    id_t* taskIdPointer = (id_t*)malloc(sizeof(id_t));
    id_t* taskIdPointer2 = (id_t*)malloc(sizeof(id_t));

    *taskIdPointer = taskId;
    *taskIdPointer2 = taskId;

    pthread_attr_t attr2;
    initThreadAttr(&attr2);

    /* Stworzenie wątków pomocniczych */
    if ((pthread_create(&params->outThread, &attr2, outReader, taskIdPointer)) != 0)
        syserr("create outReader thread");
    if ((pthread_create(&params->errThread, &attr2, errReader, taskIdPointer2)) != 0)
        syserr("create errReader thread");

    waitForExecEnd(params);

    printEnded(params);

    return NULL;
}

void* outReader(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct TaskParams* params = &taskArray[taskId].taskParams;

    FILE* f = fdopen(params->pipeFdOut[0], "r");

    // todo mutexy
    while (read_line(params->lastLineOut, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

//    printf("lastLineOut: %d\n", params->lastLineOut[0]);

    return NULL;
}

void* errReader(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct TaskParams* params = &taskArray[taskId].taskParams;

    FILE* f = fdopen(params->pipeFdErr[0], "r");

    // todo mutexy
    while (read_line(params->lastLineErr, MAX_LINE_SIZE - 1, f)) { }

    fclose(f);

    //    if (close (threadArgs->pipeFdErr[0]) == -1)
    //        syserr("Error in errReader, close (pipeFdErr[0])\n");

    return NULL;
}

void startTask(struct Task* t)
{
    initLocks(&t->taskParams);
    initThreadAttr(&t->taskParams.attr);

    id_t* taskId = (id_t*)malloc(sizeof(id_t));
    *taskId = t->taskParams.taskId;

    if ((pthread_create(&(t->mainHelperThread),
            &(t->taskParams.attr), mainHelper, taskId))
        != 0)
        syserr("create MainHelper thread");
}

void closeTask(struct Task* t)
{
    sendSignal(t, SIGKILL);
    waitForExecEnd(&t->taskParams);

    destroyLocks(&t->taskParams);

    t->taskParams.args--;
    free(t->taskParams.args); // todo może w innym miejscu
}

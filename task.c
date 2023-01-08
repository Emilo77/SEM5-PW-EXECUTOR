#include "task.h"

struct Task taskArray[MAX_TASKS];

struct Task newTask(id_t id, char* programName, char** args)
{
    struct Task newTask;

    newTask.taskId = id;
    newTask.programName = programName;
    newTask.args = args;

    memset(newTask.lastLineOut, 0, MAX_LINE_SIZE);
    memset(newTask.lastLineErr, 0, MAX_LINE_SIZE);

    return newTask;
}

void initLocks(id_t id)
{
    struct Task *task = &taskArray[id];

    if (pthread_mutex_init(&task->lockLineOut, 0) != 0)
        syserr("synchronizerInit lockLineOut failed");
    if (pthread_mutex_init(&task->lockLineErr, 0) != 0)
        syserr("synchronizerInit lockLineErr failed");
}

void destroyLocks(id_t taskId)
{
    struct Task *task = &taskArray[taskId];

    if (pthread_mutex_destroy(&task->lockLineOut) != 0)
        syserr("synchronizerDestroy lockLineOut failed");
    if (pthread_mutex_destroy(&task->lockLineErr) != 0)
        syserr("synchronizerDestroy lockLineErr failed");
}

void sendSignal(id_t taskId, int sig)
{
    struct Task *task = &taskArray[taskId];
    kill(task->execPid, sig);
}

void printStarted(id_t taskId)
{
    struct Task * task = &taskArray[taskId];
    printf("Task %d started: pid %d.\n", task->taskId, task->execPid);
}

void printOut(id_t taskId)
{
    struct Task * task = &taskArray[taskId];

    tryToLock(&task->lockLineOut);
    printf("Task %d stdout: '%s'.\n", task->taskId, task->lastLineOut);
    tryToUnlock(&task->lockLineOut);
}

void printErr(id_t taskId)
{
    struct Task *task = &taskArray[taskId];

    tryToLock(&task->lockLineErr);
    printf("Task %d stderr: '%s'.\n", task->taskId, task->lastLineErr);
    tryToUnlock(&task->lockLineErr);
}

void* printEnded(struct Task* t)
{
    if ((t->status == NOT_DONE) && !t->signal) {
        syserr("Task is not done yet.");
    }

    if (t->signal) {
        printf("Task %d ended: signalled.\n", t->taskId);

    } else {
        printf("Task %d ended: status %d.\n", t->taskId, t->status);
    }

    return NULL;
}

static void startExecProcess(struct Task* params)
{
}

void* waitForExecEnd(struct Task* task)

{
    pid_t pid = waitpid(task->execPid, &(task->status), 0);
    if (pid == -1 && errno != SIGCHLD) {
        syserr("Error in wait\n");
    }

    if (!WIFEXITED(task->status)) {
        task->signal = true;
    }
    return NULL;
}

void* mainHelper(void* arg)
{

    id_t taskId = *((id_t*)arg);
    free(arg);

    struct Task* params = &taskArray[taskId];

    /* Utworzenie łączy */
    if (pipe(params->pipeFdOut) == -1)
        syserr("Error in out pipe\n");
    if (pipe(params->pipeFdErr) == -1)
        syserr("Error in err pipe\n");

    params->execPid = fork();

    printf("pthread in mainHelper: %lu\n", params->mainHelperThread);

    switch (params->execPid) {
    /* Niepowodzenie funkcji fork */
    case -1:
        syserr("Error in fork\n");

    /* Proces-dziecko utworzone przez fork */
    case 0:
        /* Zamknięcie deskryptora na standardowe wejście */
        if (close(STDIN_FILENO) == -1)
            syserr("Error in child, close (0)\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za czytanie */
        if (close(params->pipeFdOut[0]) == -1)
            syserr("Error in child, close (pipeFdOut [0])\n");
        if (close(params->pipeFdErr[0]) == -1)
            syserr("Error in child, close (pipeFdErr [0])\n");

        /* Zamiana STDOUT i STDERR na odpowiednie deskryptory łączy */
        if (dup2(params->pipeFdOut[1], STDOUT_FILENO) == -1)
            syserr("Error in child, dup (pipeFdOut [1])\n");
        if (dup2(params->pipeFdErr[1], STDERR_FILENO) == -1)
            syserr("Error in child, dup (pipeFdErr [1])\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(params->pipeFdOut[1]) == -1)
            syserr("Error in child, close (pipeFdOut[1])\n");
        if (close(params->pipeFdErr[1]) == -1)
            syserr("Error in child, close (pipeFdErr[1])\n");

        /* Uruchomienie programu z podanymi argumentami */
        execvp(params->programName, params->args);

        /* Sytuacja, w której funkcja execvp nie powiodła się */
        syserr("Error in execvp\n");
        exit(1);

    /* Proces macierzysty */
    default:
        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(params->pipeFdOut[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");
        if (close(params->pipeFdErr[1]) == -1)
            syserr("Error in parent, close (pipeFdErr [1])\n");
    }

    id_t* taskIdPointer = (id_t*)malloc(sizeof(id_t));
    id_t* taskIdPointer2 = (id_t*)malloc(sizeof(id_t));

    *taskIdPointer = taskId;
    *taskIdPointer2 = taskId;

    /* Stworzenie wątków pomocniczych */
    if ((pthread_create(&params->outThread, NULL, outReader, taskIdPointer)) != 0)
        syserr("create outReader thread");
    if ((pthread_create(&params->errThread, NULL, errReader, taskIdPointer2)) != 0)
        syserr("create errReader thread");

    waitForExecEnd(params);

    if (pthread_join(params->outThread, NULL) != 0)
        syserr("join 1 failed");
    if (pthread_join(params->errThread, NULL) != 0)
        syserr("join 2 failed");

    printEnded(params);

    return NULL;
}

void* outReader(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct Task* params = &taskArray[taskId];

    FILE* f = fdopen(params->pipeFdOut[0], "r");

    char localBuffer[MAX_LINE_SIZE];

    // todo mutexy
    while (read_line(localBuffer, MAX_LINE_SIZE - 1, f)) {
        tryToLock(&params->lockLineOut);
        memcpy(params->lastLineOut, localBuffer, MAX_LINE_SIZE);
        tryToUnlock(&params->lockLineOut);
        printf("lastLineOut: %s\n", params->lastLineOut);
    }

    fclose(f);

    return 0;
}

void* errReader(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct Task* task = &taskArray[taskId];

    FILE* f = fdopen(task->pipeFdErr[0], "r");

    char localBuffer[MAX_LINE_SIZE];

    // todo mutexy
    while (read_line(localBuffer, MAX_LINE_SIZE - 1, f)) {
        tryToLock(&task->lockLineErr);
        memcpy(task->lastLineErr, localBuffer, MAX_LINE_SIZE);
        tryToUnlock(&task->lockLineErr);
    }

    fclose(f);

    return 0;
}

void startTask(id_t taskId)
{

    struct Task *t = &taskArray[taskId];

    initLocks(taskId);

    id_t* arg = (id_t*)malloc(sizeof(id_t));
    *arg = taskId;

    if ((pthread_create(&t->mainHelperThread, NULL, mainHelper, arg)) != 0)
        syserr("create MainHelper thread");

    printf("pthread create: %lu\n", t->mainHelperThread);
}

void closeTask(id_t taskId)
{
    struct Task * task = &taskArray[taskId];
    sendSignal(task->execPid, SIGKILL);
    //    waitForExecEnd(&t->taskParams);

    printf("pthread join: %lu\n", task->mainHelperThread);

    if (pthread_join(task->mainHelperThread, NULL) != 0)
        syserr("join mainHelper failed");

    destroyLocks(taskId);

    task->args--;
    free_split_string(task->args); // todo może w innym miejscu
}

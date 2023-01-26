#include "task.h"

struct Task taskArray[MAX_TASKS];

struct Task* newTask(id_t id, char* programName, char** args,
    struct Synchronizer* sync)
{
    struct Task* newTask = &taskArray[id];

    /* Przypisanie odpowiednich wartości początkowych */
    newTask->taskId = id;
    newTask->programName = programName;
    newTask->args = args;
    newTask->status = NOT_DONE;
    newTask->signal = false;
    newTask->sync = sync;

    /* Czyszczenie tablic */
    memset(newTask->lastLineOut, 0, MAX_LINE_SIZE);
    memset(newTask->lastLineErr, 0, MAX_LINE_SIZE);

    return newTask;
}

void initSemaphores(id_t taskId)
{
    struct Task* task = &taskArray[taskId];

    /* Ustawienie początkowej wartości semafora na 1 */
    if (sem_init(&task->lockLineOut, 0, 1) == -1)
        syserr("synchronizerInit lockLineOut failed");
    /* Ustawienie początkowej wartości semafora na 1 */
    if (sem_init(&task->lockLineErr, 0, 1) == -1)
        syserr("synchronizerInit lockLineErr failed");
    /* Ustawienie początkowej wartości semafora na 0 */
    if (sem_init(&task->lockPidWaiting, 0, 0) == -1)
        syserr("synchronizerInit lockPidWaiting failed");
}

void destroySemaphores(id_t taskId)
{
    struct Task* task = &taskArray[taskId];

    if (sem_destroy(&task->lockLineOut) == -1)
        syserr("synchronizerDestroy lockLineOut failed");
    if (sem_destroy(&task->lockLineErr) == -1)
        syserr("synchronizerDestroy lockLineErr failed");
    if (sem_destroy(&task->lockPidWaiting) == -1)
        syserr("synchronizerDestroy lockPidWaiting failed");
}

void sendSignal(id_t taskId, int sig)
{
    struct Task* task = &taskArray[taskId];
    kill(task->execPid, sig);
}

void printStarted(id_t taskId)
{
    struct Task* task = &taskArray[taskId];
    printf("Task %d started: pid %d.\n", task->taskId, task->execPid);
}

void executeOut(id_t taskId)
{
    struct Task* task = &taskArray[taskId];

    /* Wzajemne wykluczanie z wątkiem zapisującym do bufora */
    tryToLock(&task->lockLineOut);
    printf("Task %d stdout: '%s'.\n", task->taskId, task->lastLineOut);
    tryToUnlock(&task->lockLineOut);
}

void executeErr(id_t taskId)
{
    struct Task* task = &taskArray[taskId];

    /* Wzajemne wykluczanie z wątkiem zapisującym do bufora */
    tryToLock(&task->lockLineErr);
    printf("Task %d stderr: '%s'.\n", task->taskId, task->lastLineErr);
    tryToUnlock(&task->lockLineErr);
}

void* printEnded(struct Task* task)
{
    if ((task->status == NOT_DONE) && !task->signal) {
        syserr("Task is not done yet.");
    }

    /* Synchronizacja z wykonywaniem poleceń przez wątek główny */
    if (task->signal) {
        preProtocolPrinter(task->sync);
        /* Sekcja krytyczna */
        printf("Task %d ended: signalled.\n", task->taskId);

        postProtocolPrinter(task->sync);

    } else {
        preProtocolPrinter(task->sync);
        /* Sekcja krytyczna */
        printf("Task %d ended: status %d.\n", task->taskId, WEXITSTATUS(task->status));

        postProtocolPrinter(task->sync);
    }
    return NULL;
}

void* closePipesOnExec(struct Task* task)
{
    set_close_on_exec(task->pipeFdOut[0], true);
    set_close_on_exec(task->pipeFdOut[1], true);
    set_close_on_exec(task->pipeFdErr[0], true);
    set_close_on_exec(task->pipeFdErr[1], true);

    return 0;
}

void* startExecProcess(struct Task* task)
{
    task->execPid = fork();

    switch (task->execPid) {
    /* Niepowodzenie funkcji fork */
    case -1: {
        syserr("Error in fork\n");
        break;
    }

    /* Proces-dziecko utworzone przez fork */
    case 0:
        /* Zamiana STDOUT i STDERR na odpowiednie deskryptory łączy */
        if (dup2(task->pipeFdOut[1], STDOUT_FILENO) == -1)
            syserr("Error in child, dup (pipeFdOut [1])\n");
        if (dup2(task->pipeFdErr[1], STDERR_FILENO) == -1)
            syserr("Error in child, dup (pipeFdErr [1])\n");

        /* Uruchomienie programu z podanymi argumentami */
        execvp(task->programName, task->args);

        /* Sytuacja, w której funkcja execvp nie powiodła się */
        syserr("Error in execvp\n");
        exit(1);

    /* Proces macierzysty */
    default:
        /* Pozwolenie wątkowi głównemu na wypisanie,
         * że task rozpoczął się*/
        sem_post(&task->lockPidWaiting);

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(task->pipeFdOut[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");
        if (close(task->pipeFdErr[1]) == -1)
            syserr("Error in parent, close (pipeFdErr [1])\n");

        /* Argumenty nie są już potrzebne, zwolnienie zasobów  */
        task->args--;
        free_split_string(task->args);
    }
    return 0;
}

void* waitForExecEnd(struct Task* task)

{
    /* Czekanie aż proces wykonujący program zakończy się */
    pid_t pid = waitpid(task->execPid, &(task->status), 0);

    if (pid == -1 && errno != SIGCHLD) {
        syserr("Error in wait\n");
    }

    /* Oznaczenie, że task nie zakończył się
     * z poprawnym statusem */
    if (!WIFEXITED(task->status)) {
        task->signal = true;
    }

    return 0;
}

void* mainHelper(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct Task* task = &taskArray[taskId];

    /* Utworzenie łączy */
    if (pipe(task->pipeFdOut) == -1)
        syserr("Error in out pipe\n");
    if (pipe(task->pipeFdErr) == -1)
        syserr("Error in err pipe\n");

    /* Ustawienie flag dla deskryptorów */
    closePipesOnExec(task);

    /* Stworzenie procesu i rozpoczęcie programu */
    startExecProcess(task);

    /* Alokacja pamięci na argumenty przekazane do funkcji
     * obsługiwanych przez wątki */
    id_t* taskIdPointer = (id_t*)malloc(sizeof(id_t));
    id_t* taskIdPointer2 = (id_t*)malloc(sizeof(id_t));

    /* Przypisanie wartości argumentów */
    *taskIdPointer = taskId;
    *taskIdPointer2 = taskId;

    /* Stworzenie wątków pomocniczych */
    if ((pthread_create(&task->outThread, NULL, outReader, taskIdPointer)) != 0)
        syserr("create outReader thread");
    if ((pthread_create(&task->errThread, NULL, errReader, taskIdPointer2)) != 0)
        syserr("create errReader thread");

    /* Czekanie, aż stworzony proces pomocniczy zakończy się */
    waitForExecEnd(task);

    /* Czekanie na zakończenie pomocniczych wątków do
     * zapisywania ostatnich linii do buforów  */
    if (pthread_join(task->outThread, NULL) != 0)
        syserr("join 1 failed");
    if (pthread_join(task->errThread, NULL) != 0)
        syserr("join 2 failed");

    /* Wypisanie, że task zakończył się */
    printEnded(task);

    return 0;
}

void* outReader(void* arg)
{
    /* Wyciągnięcie argumentu */
    id_t taskId = *((id_t*)arg);
    free(arg);

    /* Pobranie zadania o podanym Id z tablicy zadań */
    struct Task* task = &taskArray[taskId];

    /* Otwarcie strumienia */
    FILE* f = fdopen(task->pipeFdOut[0], "r");

    /* Inicjacja chwilowego bufora do zapisywania danych */
    char localOutBuffer[MAX_LINE_SIZE];

    while (read_line(localOutBuffer, MAX_LINE_SIZE - 1, f)) {
        /* Wzajemne wykluczanie z wypisywaniem danych
         * z bufora przez wątek główny */
        tryToLock(&task->lockLineOut);
        /* Kopiowanie do właściwego bufora */
        memcpy(task->lastLineOut, localOutBuffer, MAX_LINE_SIZE);
        tryToUnlock(&task->lockLineOut);
    }

    /* Zamknięcie strumienia */
    fclose(f);

    return 0;
}

/* Funkcja analogiczna do outReader */
void* errReader(void* arg)
{
    id_t taskId = *((id_t*)arg);
    free(arg);

    struct Task* task = &taskArray[taskId];

    FILE* f = fdopen(task->pipeFdErr[0], "r");

    char localErrBuffer[MAX_LINE_SIZE];

    while (read_line(localErrBuffer, MAX_LINE_SIZE - 1, f)) {
        tryToLock(&task->lockLineErr);
        memcpy(task->lastLineErr, localErrBuffer, MAX_LINE_SIZE);
        tryToUnlock(&task->lockLineErr);
    }

    fclose(f);

    return 0;
}

void startTask(id_t taskId)
{
    struct Task* t = &taskArray[taskId];

    /* Inicjacja semaforów */
    initSemaphores(taskId);

    /* Alokacja pamięci na przekazywany argument do funkcji */
    id_t* arg = (id_t*)malloc(sizeof(id_t));
    *arg = taskId;

    /* Stworzenie głównego wątku pomocniczego do obsługi zadania */
    if ((pthread_create(&t->mainHelperThread, NULL, mainHelper, arg)) != 0)
        syserr("create MainHelper thread");
}

void closeTask(id_t taskId)
{
    struct Task* task = &taskArray[taskId];

    /* Czekanie, aż główny wątek pomocniczy zakończy się */
    if (pthread_join(task->mainHelperThread, NULL) != 0)
        syserr("join mainHelper failed");

    /* Zwolnienie zasobów */
    destroySemaphores(taskId);
}

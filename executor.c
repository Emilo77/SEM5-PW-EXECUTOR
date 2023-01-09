#include "executor.h"

void runExecutor()
{
    char inputBuffer[INPUT_BUFFER_SIZE];
    memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

    struct Synchronizer* synchronizer = malloc(sizeof(struct Synchronizer));
    synchronizerInit(synchronizer);

    char** splittedMessage = NULL;
    while (read_line(inputBuffer, INPUT_BUFFER_SIZE, stdin)) {

        /* Podzielenie linii */
        splittedMessage = split_string(inputBuffer);

        char* command = splittedMessage[0];
        char** args = splittedMessage + 1;
        bool freeResource = false;

        /* Wykonanie polecenia z zapewnieniem wykluczania
         * z wypisywaniem skończonych tasków */
        preProtocolExecutor(synchronizer);
        executeCommand(command, args, synchronizer, &freeResource);
        postProtocolExecutor(synchronizer);

        if (freeResource) {
            /* Jeżeli linia jest niepotrzebna, zwolnienie zasobów */
            free_split_string(splittedMessage);
        }
    }

    /* Zamknięcie wszystkich tasków i zamknięcie programu */
    closeAndQuit(synchronizer);
}

void executeCommand(char* command, char** args,
    struct Synchronizer* sync, bool *freeResource)
{
    *freeResource = true;
    /* Obsługa komendy run */
    if (!strcmp(command, "run")) {
        char* program_name = args[0];
        char** program_args = args;
        *freeResource = false;

        executeRun(program_name, program_args, sync);
        return;
    }

    /* Obsługa komendy sleep */
    if (!strcmp(command, "sleep")) {
        unsigned int sleep_time = atol(args[0]) * 1000;
        usleep(sleep_time);
        return;
    }

    /* Obsługa komendy quit */
    if (!strcmp(command, "quit")) {
        postProtocolExecutor(sync);
        closeAndQuit(sync);
    }

    /* Obsługa pustej linii */
    if (!strcmp(command, "")) {
        return;
    }

    long taskId = atol(args[0]);

    /* Obsługa komendy out */
    if (!strcmp(command, "out")) {
        executeOut(taskId);
        return;
    }

    /* Obsługa komendy err */
    if (!strcmp(command, "err")) {
        executeErr(taskId);
        return;
    }

    /* Obsługa komendy kill */
    if (!strcmp(command, "kill")) {
        sendSignal(taskId, SIGINT);
        return;
    }

    /* Sytuacja, w której komenda jest niepoprawna */
    syserr("Unknown command");
    postProtocolExecutor(sync);
    exit(1);
}

void executeRun(char* program, char** args,
    struct Synchronizer* sync)
{
    long newId = newTaskId();

    /* Generowanie nowego taska */
    struct Task* task = newTask(newId, program, args, sync);

    /* Rozpoczęcie taska */
    startTask(newId);

    /* Czekanie, aż otrzymamy numer stworzonego procesu */
    sem_wait(&task->lockPidWaiting);

    printStarted(newId);
}

void closeAndQuit(struct Synchronizer* sync)
{
    /* Wysłanie sygnału do każdego taska */
    for (int taskId = 0; taskId < currentTaskId; taskId++) {
        sendSignal(taskId, SIGKILL);
    }

    /* Zamykamy każdy task, zwalniamy w nim zasoby */
    for (int taskId = 0; taskId < currentTaskId; taskId++) {
        closeTask(taskId);
    }

    /* Zwalniamy zasoby do synchronizacji */
    synchronizerDestroy(sync);
    free(sync);
    exit(0);
}

int main()
{
    setbuf(stdout, NULL); // todo do usunięcia
    runExecutor();
}
#include "executor.h"

void runExecutor()
{
    memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

    synchronizerInit(&synchronizer);

    char **splittedMessage = NULL;

    while (read_line(inputBuffer, INPUT_BUFFER_SIZE, stdin)) {

        /* Podzielenie linii */
        splittedMessage = split_string(inputBuffer);

        char* command = splittedMessage[0];
        char** args = splittedMessage + 1; //todo może +1

        /* Wykonanie polecenia */
        preProtocolExecutor(&synchronizer);
        executeCommand(command, args);
        postProtocolExecutor(&synchronizer);
    }

    /* Zamknięcie wszystkich tasków i zamknięcie programu */
    closeAndQuit();
}

void executeCommand(char* command, char** args)
{
    if (!strcmp(command, "run")) {
        char* program_name = args[0];
        char** program_args = args;

        executeRun(program_name, program_args);
        return;
    }

    if (!strcmp(command, "sleep")) {
        unsigned int sleep_time = atol(args[0]) * 1000;
        usleep(sleep_time);
        free_split_string(args - 1);
        return;
    }

    if (!strcmp(command, "quit")) {
        free_split_string(args - 1);
        postProtocolExecutor(&synchronizer);
        closeAndQuit();
    }

    if (!strcmp(command, "")) {
        free_split_string(args - 1);
        return;
    }

    long taskId = atol(args[0]);

    if (!strcmp(command, "out")) {
        free_split_string(args - 1);
        executeOut(taskId);
        return;
    }

    if (!strcmp(command, "err")) {
        free_split_string(args - 1);
        executeErr(taskId);
        return;
    }

    if (!strcmp(command, "kill")) {
        free_split_string(args - 1);
        sendSignal(taskId, SIGINT);
        return;
    }

    syserr("Unknown command");
    free_split_string(args - 1);
    postProtocolExecutor(&synchronizer);
    exit(1);
}

void executeRun(char* program, char** args)
{
    long newId = newTaskId();

    struct Task *task = newTask(newId, program, args);

    startTask(newId);

    sem_wait(&task->lockPidWaiting);

    printStarted(newId);
}

void closeAndQuit()
{
    for(int taskId = 0; taskId < currentTaskId; taskId++) {
        sendSignal(taskId, SIGKILL);
    }

    for(int taskId = 0; taskId < currentTaskId; taskId++) {
        closeTask(taskId);
    }

    synchronizerDestroy(&synchronizer);
    exit(0);
}

int main()
{
    runExecutor();
}
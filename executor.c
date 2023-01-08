#include "executor.h"

void runExecutor()
{
    memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

    synchronizerInit(&synchronizer);

    while (read_line(inputBuffer, INPUT_BUFFER_SIZE, stdin)) {

        /* Podzielenie linii */
        char** splittedMessage = split_string(inputBuffer);

        char* command = splittedMessage[0];
        char** args = splittedMessage + 1; //todo może +1

        /* Wykonanie polecenia */
        preProtocolExecutor(&synchronizer);
        executeCommand(command, args);
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
        return;
    }

    if (!strcmp(command, "quit")) {
        closeAndQuit();
        exit(0);
    }

    if (!strcmp(command, "")) {
        return;
    }

    long taskId = atol(args[0]);

    if (!strcmp(command, "out")) {
        executeOut(taskId);
        return;
    }

    if (!strcmp(command, "err")) {
        executeErr(taskId);
        return;
    }

    if (!strcmp(command, "kill")) {
        sendSignal(taskId, SIGINT);
        return;
    }

    syserr("Unknown command");
    exit(1);
}

void executeRun(char* program, char** args)
{
    long newId = newTaskId();

    struct Task *task = newTask(newId, program, args);

    startTask(newId);

    tryToLock(&task->lockPidWaiting);

    printStarted(newId);
}

void closeAndQuit()
{
//     zakończ wszystkie taski
    for(int i = 0; i < currentTaskId; i++) {
        closeTask(i);
    }

    synchronizerDestroy(&synchronizer);
    exit(0);
}

int main()
{
    runExecutor();
}
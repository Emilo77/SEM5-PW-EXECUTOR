#include "executor.h"

void runExecutor()
{
    memset(inputBuffer, 0, BUFFER_SIZE);

    synchronizerInit(&synchronizer);

    while (read_line(inputBuffer, BUFFER_SIZE, stdin)) {

        /* Podzielenie linii */
        char** splittedMessage = split_string(inputBuffer);

        char* command = splittedMessage[0];
        char** args = splittedMessage + 1; //todo może +1

        /* Wykonanie polecenia */
        preProtocolExecutor(&synchronizer);
        executeCommand(command, args);

        /* Zwolnienie pamięci */
//        free_split_string(splittedMessage);
    }

    sleep(5);
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
        executeKill(taskId);
        return;
    }

    syserr("Unknown command");
    exit(1);
}

void executeRun(char* program, char** args)
{
    long new_id = newTaskId();
    struct Task task = newTask(new_id, program, args);
    taskArray[new_id] = task;

    startTask(&task);

    printStarted(&task.taskParams);
}


void executeOut(id_t task_id)
{
    struct Task task = taskArray[task_id];
    printOut(&task.taskParams);
}


void executeErr(id_t task_id)
{
    struct Task task = taskArray[task_id];
    printErr(&task.taskParams);
}


void executeKill(id_t task_id)
{
    struct Task task = taskArray[task_id];
    sendSignal(&task, SIGINT);
}


void closeAndQuit()
{
//     zakończ wszystkie taski
    for(int i = 0; i < currentTaskId; i++) {
        closeTask(&taskArray[i]);
    }

    synchronizerDestroy(&synchronizer);
    exit(0);
}

int main()
{
    runExecutor();
}
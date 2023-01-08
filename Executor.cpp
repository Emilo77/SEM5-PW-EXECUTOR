#include "Executor.h"

void Executor::executeCommand(char* command, char** args)
{
    if (!strcmp(command, "run")) {
        char* program_name = args[0];
        char** program_args = args + 1;

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

    auto task_id = atol(args[0]);

    if (!strcmp(command, "out")) {
        executeOut(task_id);
        return;
    }

    if (!strcmp(command, "err")) {
        executeErr(task_id);
        return;
    }

    if (!strcmp(command, "kill")) {
        executeKill(task_id);
        return;
    }

    syserr("Unknown command");
    exit(1);
}


void Executor::run()
{
    while (read_line(input_buffer, BUFFER_SIZE, stdin)) {

        /* Podzielenie linii */
        char** splitted_message = split_string(input_buffer);

        char* command = splitted_message[0];
        char** args = splitted_message; //todo może +1

        /* Wykonanie polecenia */
        synchronizer.preProtocolExecutor();
        executeCommand(command, args);

        /* Zwolnienie pamięci */
        free_split_string(splitted_message);
    }

    sleep(5);
    /* Zamknięcie wszystkich tasków i zamknięcie programu */
//    closeAndQuit();
}


void Executor::executeRun(char* program, char** args)
{
    auto new_id = taskArray.size();
    auto newTask = Task(new_id, program, args, synchronizer);
    taskArray.push_back(newTask);

    newTask.startTask();

    newTask.printStarted();
}


void Executor::executeOut(id_t task_id)
{
    auto task = taskArray.at(task_id);
    task.printOut();
}


void Executor::executeErr(id_t task_id)
{
    auto task = taskArray.at(task_id);
    task.printErr();
}


void Executor::executeKill(id_t task_id)
{
    auto task = taskArray.at(task_id);
    task.sendSignal(SIGINT);
}


void Executor::closeAndQuit()
{
    // zakończ wszystkie taski
    for (auto &task : taskArray) {
        task.closeTask();
    }

    synchronizer.destroy();

    exit(0);
}

int main()
{
    Executor executor = Executor();
    executor.run();
}
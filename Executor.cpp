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

        //todo zwolnij mutex do wypisywania zadań
        usleep(sleep_time);
        //todo podnieś mutex do wypisywania zadań

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

        char** splitted_message = split_string(input_buffer);

        char* command = splitted_message[0];
        char** args = splitted_message + 1;

        synchronizer.preProtocolExecutor();
        executeCommand(command, args);
        synchronizer.postProtocolExecutor();

        free_split_string(splitted_message);
    }

    closeAndQuit(); // Czekanie aż taski się wykonają itp.
}


void Executor::executeRun(char* program, char** args)
{
    auto id = idGenerator.new_id();
    auto newTask = Task(id, program, args, synchronizer);
    tasksMap.emplace(id, newTask);

    newTask.execute();

    newTask.printStarted();
}


void Executor::executeOut(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.printOut();
}


void Executor::executeErr(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.printErr();
}


void Executor::executeKill(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.sendSignal(SIGINT);
}


void Executor::closeAndQuit()
{
    // zakończ wszystkie taski
    for (auto &task : tasksMap) {
        task.second.closeTask();
    }

    synchronizer.destroy();

    //destroy mutexów

    exit(0);
}

int main()
{
    Executor executor = Executor();
    executor.run();
}
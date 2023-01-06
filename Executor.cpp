#include "Executor.h"

void Executor::execute_command(char* command, char** args)
{
    if (!strcmp(command, "run")) {
        char* program_name = args[0];
        char** program_args = args + 1;

        execute_run(program_name, program_args);
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
        close_and_quit();

        exit(0);
    }

    if (!strcmp(command, "")) {
        return;
    }

    auto task_id = atol(args[0]);

    if (!strcmp(command, "out")) {
        execute_out(task_id);
        return;
    }

    if (!strcmp(command, "err")) {
        execute_err(task_id);
        return;
    }

    if (!strcmp(command, "kill")) {
        execute_kill(task_id);
        return;
    }

    syserr("Unknown command");
    exit(1);
}


void Executor::run()
{
    memset(input_buffer, 0, BUFFER_SIZE);

    while (read_line(input_buffer, BUFFER_SIZE, stdin)) {

        char** splitted_message = split_string(input_buffer);

        char* command = splitted_message[0];
        char** args = splitted_message + 1;

        execute_command(command, args);

        free_split_string(splitted_message);
    }

    close_and_quit(); // Czekanie aż taski się wykonają itp.
}


void Executor::execute_run(char* program, char** args)
{
    auto id = idGenerator.new_id();
    auto new_task = Task(id, program, args);
    tasksMap.emplace(id, new_task);

    //new_task.start();

    new_task.print_started();
}


void Executor::execute_out(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.print_out();
}


void Executor::execute_err(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.print_err();
}


void Executor::execute_kill(id_t task_id)
{
    auto task = tasksMap.at(task_id);
    task.send_signal();
}


void Executor::close_and_quit()
{
    // zakończ wszystkie taski

    exit(0);
}


int main()
{
    Executor executor = Executor();
    executor.run();
}
#include "Executor.h"

void Executor::print_output(string s)
{
    cout << "Task T stdout: '" << s << "'.\n";
}

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

        usleep(sleep_time);
        return;
    }

    if (!strcmp(command, "quit")) {
        close_and_quit();

        exit(0);
    }

    if (!strcmp(command, "\n")) {
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

    printf("Unknown command.");
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
void Executor::close_and_quit()
{
}



void Executor::execute_run(char* program, char** args)
{
    exit(69);
}

void Executor::execute_out(id_type task_id)
{
}

void Executor::execute_err(id_type task_id)
{
}

void Executor::execute_kill(id_type task_id)
{
}

int main()
{
    Executor executor = Executor();
    executor.run();
}